#pragma once
#include <variant>
#include <Eigen/Dense>


class Shape2 {
public:
	static constexpr float maxDistance = 10e3;
	static constexpr float stepSize = 10e-3;
	Shape2(unsigned int type, Eigen::Vector3f position) {
		unsigned int x = min(position[0], maxDistance) / stepSize;
		unsigned int y = min(position[1], maxDistance) / stepSize;
		unsigned int z = min(position[2], maxDistance) / stepSize;
		//std::cout << x << " " << y << " " << z << std::endl;
		index = 0;
		index += z;
		index = index << 20;
		index += y;
		index = index << 20;
		index += x;
		index = index << 4;
		index += type;

	}
private:
	uint64_t index;

};


class Material {
protected:
	Material(unsigned int type): type(type) {};
public:
	Eigen::Vector3f color;
	unsigned int type;
	float data[4]{};
};

class Light : public Material {
public:
	Light(Eigen::Vector3f&& color) : Material(0) {
		this->color = color;
	}
};

class Lambertian: public Material {
public:
	Lambertian(Eigen::Vector3f&& color) : Material(1) {
		this->color = color;
	}
};

class Metal: public Material {
public:
	Metal(Eigen::Vector3f&& color, float refraction = 1) : Material(2) {
		this->color = color;
		data[0] = refraction;
	}
};

class Shape {
protected:
	Shape(unsigned int type, Material& material):
		transformation(Eigen::Matrix4f::Identity()), type(type), material(material){
	}
public:
	Eigen::Matrix<float,4,4,ColMajor> transformation;
	unsigned int type;
	float data[11]{};
	Material material;
};

class Circle : public Shape {
public:
	Circle(Eigen::Vector3f&& translation, float radius, Material& material): Shape(0, material) {
		transformation.block<3, 1>(0, 3) = translation;
		transformation(0, 0) = radius;
	}
};

class Box : public Shape {
public:
	Box(Eigen::Vector3f&& translation, Eigen::Vector3f&& sides, Material& material) : Shape(1, material) {
		transformation.block<3,3>(0, 0).diagonal() = 1/sides.array();
		transformation.block<3, 1>(0, 3) = transformation.block<3, 3>(0, 0) * -translation;
	}
};
