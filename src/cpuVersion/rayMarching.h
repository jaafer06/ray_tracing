#pragma once
#include "Eigen/Dense"
#include "Eigen/Geometry" 
#include <variant>
#include <vector>
#include <algorithm>
#include <optional>
#include <array>
#include "utils.h"

using namespace Eigen;
namespace RayMarching {
	constexpr float inf = std::numeric_limits<float>::max();
	struct Ray {
		Vector3f at() const {
			return origin + direction * t;
		}

		Vector3f origin;
		Vector3f direction;
		float t;
	};

	struct ImplicitSurface {
	public:
		ImplicitSurface(unsigned int materialIndex): materialIndex(materialIndex) {
			rotation = Matrix3f::Identity();
			translation = { 0, 0, 0 };
		}

		ImplicitSurface(unsigned int materialIndex, const Vector3f& translation) : materialIndex(materialIndex), translation(translation) {
			rotation = Matrix3f::Identity();
		}

		float signedDistanceFuntion(const Vector3f& point) const { throw "not implemented"; }
		inline Vector3f normalAt(const Vector3f& point) const { throw "not implemented"; }
		float hit(const Ray& ray) const { throw "not implemented"; };

		const Vector3f& getTranslation() const {
			return translation;
		}

		void setTranslatin(const Vector3f& t) {
			translation = t;
		}

		const Matrix3f& getRotation() const {
			return rotation;
		}

		void setRotation(const Matrix3f& r) {
			rotation = r;
		}

		unsigned int getMaterialIndex() const {
			return materialIndex;
		}

	protected:
		unsigned int materialIndex;
		std::array<float, 6> data;
		Matrix3f rotation;
		Vector3f translation;

	};

	namespace Visitors {
		struct SignedDistanceFunction {
			const Vector3f& point;
			template<typename Derived>
			float operator()(const Derived& derived) {
				return derived.signedDistanceFuntion(point);
			}
		};

		struct Hit {
			const Ray& ray;
			template<typename Derived>
			float operator()(Derived& derived) {
				return derived.hit(ray);
			}
		};

		struct NormalAt {
			const Vector3f& point;
			template<typename Derived>
			auto operator()(Derived& derived) {
				return derived.normalAt(point);
			}
		};

		struct MaterialIndex {
			template<typename Derived>
			unsigned int operator()(Derived& derived) const {
				return derived.getMaterialIndex();
			}
		};

	}

	struct Sphere : public ImplicitSurface {
	public:
		Sphere(unsigned int materialIndex, const Vector3f& center, float radius) : ImplicitSurface{ materialIndex, center } {
			setRadius(radius);
		}
		
		float signedDistanceFuntion(const Vector3f& point) const {
			return (point - translation).norm() - getRadius();
		}

		inline Vector3f normalAt(const Vector3f& point) const {
			return (point- translation).normalized();
		}

		float hit(const Ray& ray) const {
			const Vector3f oc = ray.origin - translation;
			const float b = oc.dot(ray.direction);
			const float c = oc.dot(oc) - getRadius() * getRadius();
			const float discriminant = b * b - c;
			if (discriminant < 0) {
				return inf;
			}
			return -b - sqrt(discriminant);
		}

		inline const float getRadius() const {
			return data[0];
		};

		void setRadius(const float radius) {
			data[0] = radius;
		};
	};


	struct Box : public ImplicitSurface {
	public:
		Box(unsigned int materialIndex, const Vector3f& center, const Vector3f dimentions, const float scale = 1) : ImplicitSurface{ materialIndex, center } {
			setDimention(dimentions);
			rotation = (AngleAxisf(1 * pi, Vector3f::UnitX()) * AngleAxisf(0.2* pi, Vector3f::UnitZ()) * AngleAxisf(0.3 * pi, Vector3f::UnitY())).toRotationMatrix();
			setScale(1/scale);
			rotation = Matrix3f::Identity();
		}

		void addR() {
			rotation *= ( AngleAxisf(0.05 * pi, Vector3f::UnitY()) ).toRotationMatrix();

		}

		float signedDistanceFuntion(const Vector3f& point) const {
			const Vector3f q = point.cwiseAbs() - getDimention();
			//return q.cwiseMax(0).norm() + std::min(q.minCoeff(), 0.f);
			return q.cwiseMax(0).norm() + std::min(q.maxCoeff(), 0.f);
		}

		float hit(const Ray& ray) const {
			const Vector3f originTransformed = getScale() * rotation * (ray.origin - translation);
			const Vector3f directionTransformed = getScale() * rotation * ray.direction;
			const Vector3f sign = originTransformed.cwiseSign();

			const Vector3f t = (0.5 - originTransformed.array() * sign.array()).cwiseQuotient(sign.array() * directionTransformed.array());
			float minT = inf;
			for (unsigned int index = 0; index < 3; ++index) {
				const Vector3f inPoint = originTransformed + directionTransformed * t(index);
				const float infNorm = inPoint.cwiseAbs().maxCoeff();
				if (infNorm < 0.5001 && infNorm > 0.4999 && minT > t(index)) {
					minT = t(index);
				}
			}
			return minT;

			//const float tOut = (0.5 - originTransformed.array() * -sign.array()).cwiseQuotient(-sign.array() * directionTransformed.array()).minCoeff();
			//const Vector3f pointOut = (originTransformed + directionTransformed * tOut);
			//const float LInfoNorm = pointOut.cwiseAbs().maxCoeff();
			//if (LInfoNorm != 0.5) {
			//	return inf;
			//}
			//return (0.5 - pointOut.array() * sign.array()).cwiseQuotient(sign.array() * -directionTransformed.array()).minCoeff();
		}

		inline Vector3f normalAt(const Vector3f& point) const {
			constexpr auto indexToAxis = [](unsigned int index) -> Vector3f {
				switch (index) {
				case 0: return { 1, 0, 0 };
				case 1: return { 0, 1, 0 };
				case 2: return { 0, 0, 1 };
				}
			};
			const Vector3f pointInMySpace = getScale() * rotation * (point - translation);
			unsigned int maxCoefIndex;
			pointInMySpace.array().cwiseAbs().maxCoeff(&maxCoefIndex);
			return rotation.transpose() * indexToAxis(maxCoefIndex) * pointInMySpace[maxCoefIndex] * 2;
			//return indexToAxis(maxCoefIndex) * std::copysignf(1., maxCoeff);
		}

		inline const Vector3f& getDimention() const {
			return Eigen::Map<const Vector3f>(data.data());
		};

		void setDimention(const Vector3f& dimention) {
			Eigen::Map<Vector3f>(data.data()) = dimention;
		};

		inline const float getScale() const {
			return data[3];
		};

		void setScale(const float scale) {
			data[3] = scale;
		};

	};

	using BasicShape = std::variant<Box, Sphere>;

	class Material {
	public:
		Material(const Vector3f& color): color(color) {}
		bool scatter(Ray& rayIn_rayOut, Vector3f& colorOut, const BasicShape& shape) {throw "Material not implemented";}
	protected:
		std::array<float,3> data;
		Vector3f color;
	};

	class Lambertian : public Material {
	public:
		Lambertian(const Vector3f& color) : Material{ color } {}

		bool scatter(Ray& rayIn_rayOut, Vector3f& colorOut, const BasicShape& shape) {
			const Vector3f contactPoint = rayIn_rayOut.at();
			const Vector3f normal = std::visit(Visitors::NormalAt{ contactPoint }, shape);
			rayIn_rayOut.direction = randomUnitVector();
			rayIn_rayOut.origin = contactPoint;
			colorOut = 0.5 * color;
			return true;
		};
	};

	class Light: public Material {
	public:
		Light(const Vector3f& color) : Material{ color } {}

		bool scatter(Ray& rayIn_rayOut, Vector3f& colorOut, const BasicShape& shape) {
			colorOut = { 5, 5, 5 };
			return false;
		};
	};

	class Metal : public Material {
	public:
		Metal(const Vector3f& color, float fuzyy = 0) : Material{ color }, fuzzy(fuzyy) {}
		bool scatter(Ray& rayIn_rayOut, Vector3f& colorOut, const BasicShape& shape) {
			const Vector3f contactPoint = rayIn_rayOut.at();
			const Vector3f normal = std::visit(Visitors::NormalAt{ contactPoint }, shape);

			rayIn_rayOut.direction = reflect(rayIn_rayOut.direction, normal).normalized();
			rayIn_rayOut.origin = contactPoint;

			colorOut = color;
			return (rayIn_rayOut.direction.dot(normal) > 0);
		};

	public:
		float fuzzy;
	};

	class DebugMaterial : public Material {
	public:
		DebugMaterial(const Vector3f& color, float fuzyy = 0) : Material{ color }, fuzzy(fuzyy) {}
		bool scatter(Ray& rayIn_rayOut, Vector3f& colorOut, const BasicShape& shape) {
			const Vector3f contactPoint = rayIn_rayOut.at();
			const Box& box = *(reinterpret_cast<const Box*>(&shape));
			const Vector3f normal = box.getRotation().transpose() * box.normalAt(contactPoint);
			rayIn_rayOut.direction = reflect(rayIn_rayOut.direction, normal).normalized();
			rayIn_rayOut.origin = contactPoint;
			colorOut = (((normal.array() + normal.cwiseSign().array().abs()) / 4)+ normal.cwiseSign().array().abs() * 0.5);
			//colorOut = colorOut.cwiseProduct(colorOut).cwiseProduct(colorOut).cwiseProduct(colorOut);
			colorOut = Vector3f{ 1, 0, 0 } * (rayIn_rayOut.direction.dot(normal) > 0);
			return false;
		};

	public:
		float fuzzy;
	};

	using Materials = std::variant<Light,Lambertian,Metal,DebugMaterial>;

	namespace Visitors {
		struct Scatter {
			Ray& rayIn_rayOut;
			Vector3f& colorOut;
			const BasicShape& shape;
			template<typename Derived>
			bool operator()(Derived& derived) {
				return derived.scatter(rayIn_rayOut, colorOut, shape);
			}
		};
	}

	struct ShapeDistance {
		const float distance;
		const BasicShape& shape;
	};

	ShapeDistance getColosestShape(const std::vector<BasicShape>& shapes, const Ray& ray) {
		float minDistance = inf;
		unsigned int minDistanceIndex = 0;
		for (unsigned int index = 0; index < shapes.size(); ++index) {
			const auto& shape = shapes[index];
			const float distance = std::visit(Visitors::Hit{ ray }, shape);
			if (distance < minDistance && distance > 0.001) {
				minDistance = distance;
				minDistanceIndex = index;
			}
		}
		return { minDistance, shapes[minDistanceIndex] };
	}

};

