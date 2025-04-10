#include <iostream>
#include <vector>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <omp.h>

using namespace cv;
using namespace std;

/// @brief Функция блюра
/// @param inputImage 
/// @param outputImage 
/// @param kernelSize 
void applyBlurFilter(const Mat &inputImage, Mat &outputImage, int kernelSize)
{
	// Проверка нечётного размера ядра
	if (kernelSize % 2 == 0)
	{
		cerr << "Error: Kernel size must be odd number. Adding 1 to make it odd." << endl;
		kernelSize += 1;
	}

	const int radius = kernelSize / 2;
	outputImage.create(inputImage.size(), inputImage.type());

	#pragma omp parallel for collapse(2) schedule(dynamic)
	for (int y = 0; y < inputImage.rows; ++y)
	{
		for (int x = 0; x < inputImage.cols; ++x)
		{
			Vec3f sum(0, 0, 0);
			int count = 0;

			// Обработка окрестности пикселя
			for (int ky = -radius; ky <= radius; ++ky)
			{
				const int ny = y + ky;
				if (ny < 0 || ny >= inputImage.rows)
					continue;

				for (int kx = -radius; kx <= radius; ++kx)
				{
					const int nx = x + kx;
					if (nx < 0 || nx >= inputImage.cols)
						continue;

					sum += Vec3f(inputImage.at<Vec3b>(ny, nx));
					++count;
				}
			}

			// Нормализация и проверка диапазона
			Vec3b result;
			if (count > 0)
			{
				result = sum / count;
				// Гарантируем корректный диапазон
				for (int c = 0; c < 3; ++c)
				{
					result[c] = saturate_cast<uchar>(result[c]);
				}
			}
			else
			{
				result = inputImage.at<Vec3b>(y, x);
			}

			outputImage.at<Vec3b>(y, x) = result;
		}
	}
}

int main(int argc, char **argv)
{
	// Проверка аргументов командной строки
	if (argc != 4)
	{
		cerr << "Usage: " << argv[0] << " <input_image> <output_image> <kernel_size>" << endl;
		cerr << "Example: " << argv[0] << " input.jpg output.jpg 5" << endl;
		return EXIT_FAILURE;
	}

	// Парсинг размера ядра
	int kernelSize;
	try
	{
		kernelSize = stoi(argv[3]);
		if (kernelSize < 1)
			throw invalid_argument("Kernel size must be positive");
	}
	catch (const exception &e)
	{
		cerr << "Error: Invalid kernel size - " << e.what() << endl;
		return EXIT_FAILURE;
	}

	// Загрузка изображения
	Mat image;
	try
	{
		image = imread(argv[1], IMREAD_COLOR);
		if (image.empty())
		{
			throw runtime_error("Failed to load image");
		}
		cout << "Image loaded: " << image.cols << "x" << image.rows
				 << ", channels: " << image.channels() << endl;
	}
	catch (const exception &e)
	{
		cerr << "Error: " << e.what() << endl;
		return EXIT_FAILURE;
	}

	// Применение фильтра
	Mat filteredImage;
	try
	{
		auto start = chrono::high_resolution_clock::now();

		applyBlurFilter(image, filteredImage, kernelSize);

		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed = end - start;
		cout << "Filter applied in " << elapsed.count() << " seconds" << endl;
	}
	catch (const exception &e)
	{
		cerr << "Error during filtering: " << e.what() << endl;
		return EXIT_FAILURE;
	}

	// Сохранение результата
	try
	{
		if (!imwrite(argv[2], filteredImage))
		{
			throw runtime_error("Failed to save image");
		}
		cout << "Image saved successfully: " << argv[2] << endl;
	}
	catch (const exception &e)
	{
		cerr << "Error: " << e.what() << endl;
		return EXIT_FAILURE;
	}

	// Отображение результатов
	const string inputWin = "Input Image (Press any key to close)";
	const string outputWin = "Filtered Image (Press any key to close)";

	namedWindow(inputWin, WINDOW_NORMAL);
	namedWindow(outputWin, WINDOW_NORMAL);

	resizeWindow(inputWin, 800, 600);
	resizeWindow(outputWin, 800, 600);

	imshow(inputWin, image);
	imshow(outputWin, filteredImage);

	waitKey(0);
	destroyAllWindows();

	return EXIT_SUCCESS;
}