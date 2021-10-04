#pragma once
#include "Eigen/Dense"
#include "Eigen/Geometry" 
#include <variant>
#include <vector>
#include <algorithm>
#include <optional>
#include <array>

using namespace Eigen;
namespace RayMarching {


	struct ImplicitSurface {
	public:
		ImplicitSurface(unsigned int materialIndex): materialIndex(materialIndex) {}

		float signedDistanceFuntion(const Vector3f& point) const {
			throw "not implemented";
		}

		inline Vector3f normalAt(const Vector3f point) {
			throw "not implemented";
		}

		unsigned int materialIndex;
		std::array<float, 6> data;

	};

	namespace Visitors {
		struct SignedDistanceFunction {
			const Vector3f& point;
			template<typename Derived>
			float operator()(const Derived& derived) {
				return derived.signedDistanceFuntion(point);
			}
		};
	}

	struct Sphere : public ImplicitSurface {
	public:
		Sphere(unsigned int materialIndex, const Vector3f& center, float radius) : ImplicitSurface{ materialIndex } {
			setCenter(center);
			setRadius(radius);
		}
		

		float signedDistanceFuntion(const Vector3f& point) const {
			return (point - getCenter()).norm() - getRadius();
		}

		inline Vector3f normalAt(const Vector3f& point) {
			return (point-getCenter()).normalized();
		}

		inline const Vector3f getCenter() const {
			return Eigen::Map<const Vector3f>(data.data());
		};

		void setCenter(const Vector3f& center) {
			Eigen::Map<Vector3f>(data.data()) = center;
		};

		inline const float getRadius() const {
			return data[3];
		};

		void setRadius(const float radius) {
			data[3] = radius;
		};
	};


	struct Box : public ImplicitSurface {
	public:
		Box(unsigned int materialIndex, const Vector3f& center, const Vector3f dimentions) : ImplicitSurface{ materialIndex } {
			setCenter(center);
			setDimention(dimentions);
		}

		float signedDistanceFuntion(const Vector3f& point) const {
			const Vector3f q = point.cwiseAbs() - getDimention();
			//return q.cwiseMax(0).norm() + std::min(q.minCoeff(), 0.f);
			return q.cwiseMax(0).norm() + std::min(q.maxCoeff(), 0.f);
		}

		inline Vector3f normalAt(const Vector3f& point) {
			constexpr auto indexToAxis = [](unsigned int index) -> Vector3f {
				switch (index) {
					case 0: return { 1, 0, 0 };
					case 1: return { 0, 1, 0 };
					case 2: return { 0, 0, 1 };
				}
			};

			const unsigned int minCoef = (1-point.array().cwiseAbs()).minCoeff();
			const int sign = std::signbit(point(minCoef)) * 2 - 1;
			return indexToAxis(minCoef)* sign;
		}

		inline const Vector3f getCenter() const {
			return Eigen::Map<const Vector3f>(data.data());
		};

		void setCenter(const Vector3f& center) {
			Eigen::Map<Vector3f>(data.data()) = center;
		};

		inline const Vector3f getDimention() const {
			return Eigen::Map<const Vector3f>(data.data()+3);
		};

		void setDimention(const Vector3f& dimention) {
			Eigen::Map<Vector3f>(data.data()+3) = dimention;
		};
	
	};

	using BasicShape = std::variant<Box, Sphere>;

	struct Material {
		std::optional<Ray> scatter(const Ray& rayIn, const BasicShape& shape) {
			
		}
	};

	struct Ray {
		const Vector3f origin;
		const Vector3f direction;
		float t;

		Vector3f at() const {
			return origin + direction * t;
		}

		Vector3f at(float tPrime) const {
			return origin + direction * tPrime;
		}

		void advance(float distance) {
			t +=  distance;
		}

	};


	using HitResult = std::optional<const BasicShape>;

	struct ShapeDistance {
		const float distance;
		const BasicShape& shape;
	};

	ShapeDistance getColosestShape(const std::vector<BasicShape>& shapes, const Vector3f& currentPosition) {
		float minDistance = std::numeric_limits<float>::max();
		unsigned int minDistanceIndex = 0;
		for (unsigned int index = 0; index < shapes.size(); ++index) {
			const auto& shape = shapes[index];
			const float distance = std::visit(Visitors::SignedDistanceFunction{ currentPosition }, shape);
			if (distance < minDistance) {
				minDistance = distance;
				minDistanceIndex = index;
			}
		}
		return { minDistance, shapes[minDistanceIndex] };
	}

	HitResult march(const std::vector<BasicShape>& shapes, Ray& ray, const float maxDepth, unsigned int maxMarchingSteps = 10) {
		do {
			const Vector3f currentPosition = ray.at();
			const ShapeDistance& closestShapeDistance = getColosestShape(shapes, currentPosition);

			if (closestShapeDistance.distance < 0.01 && closestShapeDistance.distance > -0.01) {
				return { closestShapeDistance.shape };
			} else if (closestShapeDistance.distance > maxDepth || maxMarchingSteps <= 0) {
				return { std::nullopt };
			}
			//std::cout << closestShapeDistance.distance << std::endl;

			ray.advance(closestShapeDistance.distance);
			--maxMarchingSteps;
		} while (true);

	}

};

