#ifndef ALTITUDE_H
#define ALTITUDE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Matrix.h"
#include <iostream>
#include <vector>
#include <random>

class Surface
{
	public:
	Surface(const std::string& pathAltitude):altitude(pathAltitude), numberOfAttributes(4),currentRowIndices(NULL), lastRowIndices(NULL),texture(0)
	{
		loadVertexAndIndex();
	}
	Surface(const std::string& pathAltitude, const std::string& pathLava, const std::string& pathTemperature):altitude(pathAltitude), lava(pathLava), temperature(pathTemperature), numberOfAttributes(4),currentRowIndices(NULL), lastRowIndices(NULL), texture(0)
	{
		loadVertexAndIndex();
	}

	float getMaxHeight()
	{
		return altitude.getMaxValue();
	}

	float getMinHeight()
	{
		return altitude.getMinValue();
	}

	float getDropHeight()
	{
		return altitude.getMaxValue() - altitude.getMinValue();
	}

	unsigned int getRows()
	{
		return altitude.getRows();
	}

	unsigned int getColumns()
	{
		return altitude.getColumns();
	}

	float getCellSize()
	{
		return altitude.getCellSize();
	}

	void loadTexture(char const * path)
	{
	    glGenTextures(1, &texture);

	    int width, height, nrComponents;
	    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	    if (data)
	    {
	        GLenum format;
	        if (nrComponents == 1)
	            format = GL_RED;
	        else if (nrComponents == 3)
	            format = GL_RGB;
	        else if (nrComponents == 4)
	            format = GL_RGBA;

	        glBindTexture(GL_TEXTURE_2D, texture);
	        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	        glGenerateMipmap(GL_TEXTURE_2D);

	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	        stbi_image_free(data);
	    }
	    else
	    {
	        std::cout << "Texture failed to load at path: " << path << std::endl;
	        stbi_image_free(data);
	    }
	}
		
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indicesEBO;
	unsigned int texture;
	unsigned int VAO;
	private:
		Matrix altitude;
		Matrix lava;
		Matrix temperature;

		int* currentRowIndices;
		int* lastRowIndices;

		unsigned int numberOfAttributes;
		
		void loadVertexAndIndex()
		{
			inizializeIndexRow();

			for(int i=0; i<altitude.getRows(); i++)
			{
				for(int j=0; j<altitude.getColumns(); j++)
				{
					if(altitude.isHoleCell(i,j))
					{
						currentRowIndices[j + 1] = -1;
						if(j == 0)
						{
							currentRowIndices[j] = -1;
						}
						continue;
					}

					int topLeftIndex=-1;
					int topRightIndex=-1;
					int bottomLeftIndex=-1;
					int bottomRightIndex=-1;
					glm::vec3 topLeftVertex;
					glm::vec3 topRightVertex;
					glm::vec3 bottomLeftVertex;
					glm::vec3 bottomRightVertex;

					unsigned currentIndex=vertices.size()/numberOfAttributes;

					bool noTopLeftVertex = (lastRowIndices[j] == -1);
					bool noTopRightVertex = (i==0 || lastRowIndices[j + 1] == -1);
					bool noBottomLeftVertex = (j == 0 || currentRowIndices[j] == -1);
						
					float altitudeCell = altitude.getValue(i,j);
					float lavaThickness = 0.0f;
					if(lava.isLoaded())
					{
						lavaThickness = lava.getValue(i,j);
					}

					//Top-left vertex
					if(noTopLeftVertex)
					{
						//No Top-left vertex: generate one
						topLeftVertex = generateVertex(i, j, altitudeCell, lavaThickness);

						topLeftIndex = currentIndex;
						currentIndex++;
					}
					else
					{
						topLeftIndex = lastRowIndices[j];
						topLeftVertex = vertices[topLeftIndex*numberOfAttributes];
					}

					//Bottom-left vertex
					if(noBottomLeftVertex)
					{
						//No Bottom-left vertex: generate one
						bottomLeftVertex = generateVertex(i+1, j, altitudeCell, lavaThickness);
						bottomLeftIndex = currentIndex;
						currentIndex++;

						currentRowIndices[j] = bottomLeftIndex;
					}
					else
					{
						bottomLeftIndex = currentRowIndices[j];
						bottomLeftVertex = vertices[bottomLeftIndex*numberOfAttributes];
					}

					//Top-Right vertex
					if(noTopRightVertex)
					{
						//No Top-Right vertex: generate one
						topRightVertex= generateVertex(i, j+1, altitudeCell, lavaThickness);
						topRightIndex = currentIndex;
						currentIndex++;

						lastRowIndices[j + 1] = topRightIndex;
					}
					else
					{
						topRightIndex = lastRowIndices[j + 1];
						topRightVertex = vertices[topRightIndex*numberOfAttributes];
					}

					//Bottom-Right vertex
					bottomRightVertex = generateVertex(i+1, j+1, altitudeCell, lavaThickness);
					bottomRightIndex = currentIndex;
					currentIndex++;
					currentRowIndices[j + 1] = bottomRightIndex;

					glm::vec3 normal = generateNormal(bottomLeftVertex - topLeftVertex, topRightVertex - topLeftVertex);

					//Redvalue
					glm::vec3 redColor;
					if(temperature.isLoaded())
					{
						redColor = glm::vec3(computeRedValue(i, j), 0.0f, 0.0f);
					}

					//add vertices and attributes
					if(noTopLeftVertex)
					{
						//Tex coordinate
						glm::vec3 texCoord = computeTexCoord(topLeftVertex);
						addVertex(topLeftVertex, normal, redColor, texCoord);
					}

					if(noBottomLeftVertex)
					{
						glm::vec3 texCoord = computeTexCoord(bottomLeftVertex);
						addVertex(bottomLeftVertex, normal, redColor, texCoord);
					}

					if(noTopRightVertex)
					{
						glm::vec3 texCoord = computeTexCoord(topRightVertex);
						addVertex(topRightVertex, normal, redColor, texCoord);
					}

					glm::vec3 texCoord = computeTexCoord(bottomRightVertex);
					addVertex(bottomRightVertex, normal, redColor, texCoord);

					//generate first triangle
					indicesEBO.push_back(topLeftIndex);
					indicesEBO.push_back(bottomLeftIndex);
					indicesEBO.push_back(topRightIndex);

					//generate second triangle
					indicesEBO.push_back(bottomLeftIndex);
					indicesEBO.push_back(topRightIndex);
					indicesEBO.push_back(bottomRightIndex);
				}

				//swap current and last row pointers
				int * tmp=currentRowIndices;
				currentRowIndices=lastRowIndices;
				lastRowIndices=tmp;
			}
		}

		void inizializeIndexRow()
		{	
			currentRowIndices = new int[altitude.getColumns() + 1];
			lastRowIndices = new int[altitude.getColumns() + 1];
			for (int i = 0; i < altitude.getColumns() + 1; i++)
			{
				currentRowIndices[i] = -1;
				lastRowIndices[i] = -1;
			}
		}

		glm::vec3 generateVertex(const int zTimesCellSize, const int xTimesCellSize, const float height, const float lavaThickness)
		{
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;

			x = xTimesCellSize * altitude.getCellSize();
			z = zTimesCellSize * altitude.getCellSize();
			y = height - altitude.getMinValue() + lavaThickness;

			glm::vec3 vertex = glm::vec3(x, y, z);

			return vertex;
		}

		void addVertex(const glm::vec3& vertex, const glm::vec3& normal, const glm::vec3& redValue, const glm::vec3& tex)
		{
			vertices.push_back(vertex);
			vertices.push_back(normal);
			vertices.push_back(redValue);
			vertices.push_back(tex);
		}

		glm::vec3 generateNormal(const glm::vec3& first, const glm::vec3& second)
		{
			return glm::normalize(glm::cross(glm::normalize(first), glm::normalize(second)));
		}

		glm::vec3 computeTexCoord(const glm::vec3& vertex)
		{
			const float maxX = (altitude.getColumns() + 1)
					* altitude.getCellSize();
			const float maxZ = (altitude.getRows() + 1)
					* altitude.getCellSize();
			
			float x = 0.0f;
			x = vertex.x / maxX;

			float y = 0.0f;
			y = 1 - (vertex.z) / maxZ;

			return glm::vec3(x, y, 0.0f);
		}

		float computeRedValue(int i, int j)
		{
			float redValue = temperature.getValue(i, j);
			if (redValue != 0)
			{
				redValue -= temperature.getMinValue();
				redValue /= (temperature.getMaxValue() - temperature.getMinValue());
			}
			return redValue;
		}

		void printRowIndices()
		{
			std::cout<<"LAST ROW: \n";
			for(int x=0; x<altitude.getColumns()+1; x++)
			{
				std::cout<<lastRowIndices[x]<<" ";
			}
			std::cout<<"\n";

			std::cout<<"CURRENT ROW: \n";
			for(int x=0; x<altitude.getColumns()+1; x++)
			{
				std::cout<<currentRowIndices[x]<<" ";
			}
			std::cout<<"\n\n";
		}
};

#endif
