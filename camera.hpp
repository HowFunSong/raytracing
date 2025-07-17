#ifndef CAMERA_H
#define CAMERA_h

#include "hittable.hpp"
#include "material.hpp"

class camera{
  public :
    double aspect_ratio   = 16.0 / 9.0;
    double image_width    = 100;
    int samples_per_pixel = 10;
    int max_depth         = 50;

    double vfov = 20; // degrees

    point3 lookfrom = point3(0, 0, 0);
    point3 lookat   = point3(0, 0, -1);
    vec3   vup      = vec3(0,1,0);   

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

    void render (const hittable& world){

        initialize();
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        
        //可以用thread 然後結合矩陣先把資料存在memory
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color (0.0, 0.0, 0.0);
                for (int sample = 0 ; sample < samples_per_pixel ; sample++){
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
    
                write_color(std::cout, pixel_sample_scale * pixel_color);
            }
        }

        //最後在把圖片寫入記憶體

        std::clog << "Done.\n\n";
    }

  private :
    int image_height;            // Rendered image height
    double pixel_sample_scale;   // Color scale factor for a sum of pixel samples
    point3 pixel00_loc;          // Location of pixel 0, 0
    point3 center;               // Camera center
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;
    vec3   u, v, w;              // Camera frame basis vectors
    vec3 defocus_disk_u;         // Defocus disk horizontal radius
    vec3 defocus_disk_v;         // Defocus disk vertical radius

    //初始化相機參數
    void initialize(){
        // Calculate the image height, and ensure that it's at least 1.
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        pixel_sample_scale = 1.0 / samples_per_pixel ;
        center = lookfrom;

        // Camera parameter setting
        // auto focal_length = (lookat - lookfrom).length();

        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2); // half of viewport height / focal_length = tan(theta/2)
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // u : unit vector pointing to camera right
        // v : unit vector pointing to camera up
        // w : unit vector pointing opposite the view direction 
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = viewport_width * u;;
        auto viewport_v = viewport_height * -v;
      
        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;
        
        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center -  (focus_dist * w)  - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;// 透鏡右手側
        defocus_disk_v = v * defocus_radius;//  透鏡上方側
    }
    
    ray get_ray(int i, int j) const{
 
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.
        vec3 offset = sample_square();
        point3 pixel_sample =   pixel00_loc 
                                + ((i + offset.x()) * pixel_delta_u) 
                                + ((j + offset.y()) * pixel_delta_v);

        point3 ray_origin = defocus_angle < 0 ? center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;
        
        double ray_time = random_double();
        
        return ray(ray_origin, ray_direction, ray_time);

    }
    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }
    

    point3 defocus_disk_sample()const{
        // Returns a random point in the camera defocus disk.
        point3 p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }


    color ray_color(const ray& r, int depth,const hittable& world) const{
        if (depth <= 0) return color(0, 0, 0);

        hit_record rec;
        //if hit 
        if (world.hit(r, interval(0.001, infinity), rec)) { //用bvh優化，原本對整體物件進行線性搜索O(n) -> O(log n)
  
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth-1 , world);
            else
                return color (0, 0, 0);
            // return 0.5 * (rec.normal + color(1,1,1));
        }

        // background
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);

    }
};

#endif