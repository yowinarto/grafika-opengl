#ifndef IMAGELOADER_H
#define IMAGELOADER_H
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


class ImageLoader {
public:
	ImageLoader(const char* filePath) {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		data = stbi_load(filePath, &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "Failed to load texture" << std::endl;
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
	}

	void use() {
		glBindTexture(GL_TEXTURE_2D, texture);
	}

private:
	unsigned int texture;
	int width, height, nrChannels;
	unsigned char* data;
};
#endif