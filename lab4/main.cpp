#include <iostream>
#include <vector>
#include <cmath>
#include <mpi.h>

// Параметры задачи
const double a = 100000.0;
const double epsilon = 0.000001;
const int max_iters = 20;
const double init_value = 0.0;

// Область расчета
const double start[3] = {-1.0, -1.0, -1.0};
const double size[3] = {2.0, 2.0, 2.0};
int grid_size[3] = {480, 480, 480};

// Параметры сетки
struct Grid
{
    double step[3];
    int local_layers;

    Grid(int procs)
    {
        step[0] = size[0] / (grid_size[0] - 1);
        step[1] = size[1] / (grid_size[1] - 1);
        step[2] = size[2] / (grid_size[2] - 1);
        local_layers = grid_size[2] / procs;
    }
};

// Точное решение
double exact(double x, double y, double z)
{
    return x * x + y * y + z * z;
}

// Правая часть уравнения
double source(double x, double y, double z)
{
    return 6.0 - a * exact(x, y, z);
}

// Вычисление нового значения
double calculate_new_value(const std::vector<std::vector<double>> &grid,
                           int layer, int point, const Grid &params, int rank)
{
    const double hx = params.step[0], hy = params.step[1], hz = params.step[2];
    const double coef = 2.0 / (hx * hx) + 2.0 / (hy * hy) + 2.0 / (hz * hz) + a;

    const int nx = grid_size[0], ny = grid_size[1];
    double x = start[0] + (point % nx) * hx;
    double y = start[1] + ((point / nx) % ny) * hy;
    double z = start[2] + (layer + rank * params.local_layers) * hz;

    return ((grid[layer][point + 1] + grid[layer][point - 1]) / (hx * hx) +
            (grid[layer][point + nx] + grid[layer][point - nx]) / (hy * hy) +
            (grid[layer + 1][point] + grid[layer - 1][point]) / (hz * hz) -
            source(x, y, z)) /
           coef;
}

// Обмен границами между процессами
void exchange_borders(std::vector<std::vector<double>> &grid,
                      int rank, int procs, MPI_Request *req)
{
    int points_per_layer = grid_size[0] * grid_size[1];

    if (rank > 0)
    {
        MPI_Isend(&grid[1][0], points_per_layer, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[0]);
        MPI_Irecv(&grid[0][0], points_per_layer, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &req[1]);
    }
    if (rank < procs - 1)
    {
        MPI_Isend(&grid[grid.size() - 2][0], points_per_layer, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[2]);
        MPI_Irecv(&grid.back()[0], points_per_layer, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &req[3]);
    }
}

// Инициализация сетки
void init_grid(std::vector<std::vector<double>> &grid, int rank, const Grid &params)
{
    int nx = grid_size[0], ny = grid_size[1];

    for (int layer = 0; layer < grid.size(); ++layer)
    {
        for (int point = 0; point < grid[layer].size(); ++point)
        {
            int x_idx = point % nx;
            int y_idx = (point / nx) % ny;
            double z = start[2] + (layer + rank * params.local_layers) * params.step[2];

            if (x_idx == 0 || x_idx == nx - 1 || y_idx == 0 || y_idx == ny - 1 ||
                layer == 0 || layer == grid.size() - 1)
            {
                grid[layer][point] = exact(start[0] + x_idx * params.step[0],
                                           start[1] + y_idx * params.step[1], z);
            }
            else
            {
                grid[layer][point] = init_value;
            }
        }
    }
}

// Вычисление максимальной ошибки
double compute_error(const std::vector<std::vector<double>> &curr,
                     const std::vector<std::vector<double>> &prev)
{
    double max_diff = 0.0;
    for (size_t l = 0; l < curr.size(); ++l)
    {
        for (size_t p = 0; p < curr[l].size(); ++p)
        {
            max_diff = std::max(max_diff, std::abs(curr[l][p] - prev[l][p]));
        }
    }

    double global_max;
    MPI_Allreduce(&max_diff, &global_max, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    return global_max;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int procs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Grid params(procs);
    std::vector<std::vector<double>> grid(params.local_layers,
                                          std::vector<double>(grid_size[0] * grid_size[1]));
    auto old_grid = grid;

    init_grid(grid, rank, params);

    double error = 1.0;
    int iter = 0;
    MPI_Request req[4];

    double start_time = MPI_Wtime();

    while (error > epsilon && iter < max_iters)
    {
        old_grid = grid;

        // Обновление граничных слоев
        for (int l = 1; l < grid.size() - 1; ++l)
        {
            for (int p = grid_size[0] + 1; p < grid[l].size() - grid_size[0] - 1; ++p)
            {
                grid[l][p] = calculate_new_value(old_grid, l, p, params, rank);
            }
        }

        exchange_borders(grid, rank, procs, req);

        // Обновление внутренних точек
        for (int l = 2; l < grid.size() - 2; ++l)
        {
            for (int p = 2 * grid_size[0] + 1; p < grid[l].size() - 2 * grid_size[0] - 1; ++p)
            {
                grid[l][p] = calculate_new_value(old_grid, l, p, params, rank);
            }
        }

        MPI_Status status[4];
        if (rank > 0)
            MPI_Waitall(2, req, status);
        if (rank < procs - 1)
            MPI_Waitall(2, req + 2, status + 2);

        error = compute_error(grid, old_grid);
        if (rank == 0)
        {
            std::cout << "Iteration " << iter << ", Error: " << error << std::endl;
        }
        iter++;
    }

    if (rank == 0)
    {
        std::cout << "Total time: " << MPI_Wtime() - start_time << " seconds\n";
    }

    MPI_Finalize();
    return 0;
}