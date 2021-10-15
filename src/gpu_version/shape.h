#pragma once
#include <variant>
#include <Eigen/Dense>

class Material {
protected:
	Material(unsigned int type): type(type) {};
public:
	Eigen::Vector3f color;
	unsigned int type;
	float data[4]{};
};

class Lambertian: public Material {
public:
	Lambertian(Eigen::Vector3f&& color) : Material(1) {
		this->color = color;
	}
};

class Shape {
protected:
	Shape(unsigned int type, Material& material):
		transformation(Eigen::Matrix4f::Identity()), type(type), material(material){
	}
public:
	Eigen::Matrix<float,4,4,RowMajor> transformation;
	unsigned int type;
	float data[11]{};
	Material material;
};

class Circle : public Shape {
public:
	Circle(Eigen::Vector3f&& translation, Material& material): Shape(1, material) {
		transformation.block<3, 1>(0, 3) = translation;
	}
};

class Box : public Shape {
public:
	Box(Eigen::Vector3f&& translation, Material& material) : Shape(1, material) {
		transformation.block<3, 1>(0, 3) = translation;
	}
};