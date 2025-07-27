#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.hpp"
#include "texture.hpp"

class  material{
  public:
    virtual ~material() = default;
    virtual color emitted(double u, double v, const point3& p)const{
      return color(0, 0, 0);
    }

    virtual bool scatter(
      const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    )const{
       return false; 
    }
    virtual double scattering_pdf(const ray&r_in, const hit_record& rec, const ray& scattered) 
    const{
      return 0;  
    }
};


class lambertian : public material{
  public :

    // 1. 傳入color -> 呼叫 texture建構子
    lambertian(const color& albedo) : tex(make_shared<solid_color>(albedo)){}
    // 2. 傳入 texture 
    lambertian(shared_ptr<texture> tex) : tex(tex){};

    bool scatter ( const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) 
    const override {
        //vec3 scatter_dir = rec.normal + random_unit_vector();
        vec3 scatter_dir = random_on_hemisphere(rec.normal);
        // Catch degenerate scatter direction
        if (scatter_dir.near_zero())
            scatter_dir = rec.normal;
            
        scattered = ray(rec.p, scatter_dir, r_in.time());
        attenuation = tex->value (rec.u, rec.v, rec.p);

        return true;
    }

    double scattering_pdf(const ray&r_in, const hit_record& rec, const ray& scattered) 
    const override{
        // auto cos_theta = dot(rec.normal, unit_vector(scattered.direction()));
        // return cos_theta < 0 ? 0 : cos_theta/pi;
        return 1 / (2*pi);

    }


  private :
    color albedo;
    shared_ptr<texture> tex;
};

class metal : public material{
  public : 
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz){};
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
      // 要先判斷是否擊中hit -> rec 有資訊 -> 才使用這個function;
      vec3 reflected_dir = reflect(r_in.direction(), rec.normal) + (fuzz * random_unit_vector());
      scattered = ray(rec.p, reflected_dir, r_in.time());
      attenuation = albedo;
      return dot(scattered.direction(), rec.normal) > 0;

    }

  private :
    color albedo ;  
    double fuzz;

};

class dielectric : public material {
  public :
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, direction, r_in.time());
        return true;
    }

  private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double refraction_index;
    static double reflectance(double cosine, double refraction_index) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1 - cosine),5);
    }
};

class diffuse_light :public material{
  public :
    diffuse_light(shared_ptr<texture> tex) : tex(tex){};
    diffuse_light(const color& emit) :tex(make_shared<solid_color>(emit)) {};
    
    color emitted(double u ,double v, const point3& p) const override{
      return tex->value(u, v, p);
    }
  private :
    shared_ptr<texture> tex ;

};

class isotropic : public material {
  public:
    isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
    isotropic(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        scattered = ray(rec.p, random_unit_vector(), r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }

  private:
    shared_ptr<texture> tex;
};
#endif