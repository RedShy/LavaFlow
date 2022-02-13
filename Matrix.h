#ifndef MATRIX_H
#define MATRIX_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cfloat>
#include <limits>

class Matrix
{
	public:

		Matrix():matrix(NULL),columns(0),rows(0),noDataValue(0),cellSize(0), 
		maxValue(-std::numeric_limits<float>::max()), minValue(std::numeric_limits<float>::max())
		{}

		Matrix(const std::string& path):matrix(NULL),columns(0),rows(0),noDataValue(0),cellSize(0), 
		maxValue(-std::numeric_limits<float>::max()), minValue(std::numeric_limits<float>::max())
		{
			loadFile(path);
		}

		~Matrix()
	    {
	        for (int i = 0; i < rows; ++i) {
	            delete [] matrix[i];
	        }
	        delete [] matrix;
	    }

	    bool isLoaded()
	    {
	    	return matrix != NULL;
	    }

	    void loadFile (const std::string& path)
	    {
	        std::string line;
	        std::ifstream myfile (path);
	        int i = 0;
	        int row=0;

	        if (myfile.is_open())
	        {
	            while ( std::getline (myfile,line) )
	            {
	                std::vector<std::string> sep = split(line, ' ');
	                switch (i) 
	                {
		                case 0:
		                    columns = std::stoi(sep[1]);
		                    break;
		                case 1:
		                    rows= std::stoi(sep[1]);
		                    matrix = new float* [rows];
		                    break;
		                case 2:
		                    //xllcorner
		                    break;
		                case 3:
		                    //yllcorner
		                    break;
		                case 4:
		                    cellSize = std::stof(sep[1]);
		                    break;
		                case 5:
		                    noDataValue = std::stof(sep[1]);
		                    break;
		                default:
		                    matrix[row] = new float[columns];

		                    for(int col=0; col<columns; col++)
		                    {
		                        matrix[row][col] = std::stof(sep[col]);
		                        
		                        if(matrix[row][col] != noDataValue)
		                        {
			                        if(matrix[row][col] > maxValue)
			                        {
			                        	maxValue = matrix[row][col];
			                        }

			                        if(matrix[row][col] < minValue)
			                        {
			                        	minValue = matrix[row][col];
			                        }
		                   		}
		                    }
		                    row++;
		                    break;
	                }

	                i++;

	            }
	            myfile.close();
	        }
	    }

	    bool isHoleCell(const int row, const int column)
	    {
	    	assert(matrix!=NULL && row<rows && column<columns);

	    	return matrix[row][column] == noDataValue;
	    }

		void printMatrix()
		{
			std::cout<<"columns: "<<columns<<" rows: "<<rows<<" NOVALUE: "<<noDataValue<<"\n";
			for(int i=0; i<rows; i++)
			{
				for(int j=0; j<columns; j++)
				{
					std::cout<<matrix[i][j]<<" ";
				}
				std::cout<<"\n";
			}
		}

		float getCellSize()
	    {
	        return cellSize;
	    }

	    float getMaxValue()
	    {
	        return maxValue;
	    }

	    float getMinValue()
	    {
	        return minValue;
	    }

	    int getColumns()
	    {
	        return columns;
	    }

	    int getRows()
	    {
	        return rows;
	    }

	    const float getValue(const int i, const int j)
	    {
	    	assert(matrix!=NULL && i<rows && j<columns);
	    	return matrix[i][j];
	    }

	private:
		float ** matrix;
		int columns;
		int rows;
		float noDataValue;
		float cellSize;
		float maxValue;
		float minValue;

		std::vector<std::string> split(std::string str, char delimiter) 
		{
	        std::vector<std::string> internal;
	        std::stringstream ss(str);
	        std::string tok;

	        while(std::getline(ss, tok, delimiter)) 
	        {
	            internal.push_back(tok);
	        }
	        return internal;
    	}
};

#endif