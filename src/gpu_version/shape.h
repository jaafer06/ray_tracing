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


class DebugMaterial : public Material {
public:
	DebugMaterial(Eigen::Vector3f&& color) : Material(100) {
		this->color = color;
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
		transformation.block<3, 3>(0, 0).diagonal() = sides;
		transformation.block<3, 1>(0, 3) = translation;
	}
};

class Triangle : public Shape {
public:
	Triangle(Eigen::Vector3f&& p1, Eigen::Vector3f&& p2, Eigen::Vector3f&& p3, Material& material) : Shape(2, material) {
		transformation.col(0).head(3) = p1;
		transformation.col(1).head(3) = p2;
		transformation.col(2).head(3) = p3;
	}
};

class SimpleTriangle {
public:
	SimpleTriangle(Eigen::Vector3f&& p1, Eigen::Vector3f&& p2, Eigen::Vector3f&& p3) {
		position.head(3) = p1;

		const Eigen::Vector3f v1 = p2 - p1;
		const Eigen::Vector3f v2 = p3 - p1;
		const Eigen::Vector3f a = v1.cross(v2).normalized();
		float beta = a.dot(p1);
		this->a = a;
		this->beta = beta;

		Vector3f m1 = v1 - ((v1.dot(v2) / v2.dot(v2)) * v2);
		m1 = m1 / m1.dot(v1);

		Vector3f m2 = v2 - ((v1.dot(v2) / v1.dot(v1)) * v1);
		m2 = m2 / m2.dot(v2);

		this->m1.head(3) = m1;
		this->m2.head(3) = m2;

	}
private:
	Eigen::Vector4f position;
	Eigen::Vector3f a;
	float beta;
	Eigen::Vector4f m1;
	Eigen::Vector4f m2;
};