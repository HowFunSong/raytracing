#ifndef CAMERA_H
#define CAMERA_H
#include <iostream>
#include <thread>
#include <fstream>
#include <memory>
#include "hittable.hpp"
#include "material.hpp"
#include <indicators/dynamic_progress.hpp>
#include <indicators/progress_bar.hpp>
using namespace indicators;


class camera{
  public :
    double aspect_ratio   = 16.0 / 9.0;
    double image_width    = 100;
    int samples_per_pixel = 10;
    int max_depth         = 50;
    color background;

    double vfov = 20; // degrees

    point3 lookfrom = point3(0, 0, 0);
    point3 lookat   = point3(0, 0, -1);
    vec3   vup      = vec3(0,1,0);   

    double defocus_angle = 0; // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus
    unsigned int  n_threads;
    void render_multi_threads (const hittable& world){

        initialize();

        const int W = image_width;
        const int H = image_height;

        std::vector<color> framebuffer(W * H);
        // 決定使用的執行緒數
        n_threads = std::thread::hardware_concurrency();
        if (n_threads == 0) n_threads = 4;  
        std::clog << "number of thread in use : " << n_threads << "\n" << std::flush;
        
        // 把 H 列平分給各執行緒
        int rows_per_thread = H / n_threads;

        std::vector<std::unique_ptr<ProgressBar>> bars_vec;
            for (unsigned t = 0; t < n_threads; ++t) {
                int rows = rows_per_thread;

                char buf[16] ;
                std::snprintf(buf, sizeof(buf), "Worker %2u: ", t);

                bars_vec.emplace_back(std::make_unique<ProgressBar>(
                    option::Stream{std::clog},
                    option::BarWidth{50},
                    option::MaxProgress{rows},
                    option::PrefixText{buf},
                    indicators::option::FontStyles{
                        std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}));
            }

        DynamicProgress<ProgressBar> bars(std::move(bars_vec[0]));
        for (unsigned t = 1; t < n_threads; ++t)
            bars.push_back(std::move(bars_vec[t]));
        bars.set_option(option::HideBarWhenComplete{false});

        auto worker = [&](int start_row, int end_row, int tid) {
            
            int total = end_row - start_row;

            for (int j = start_row; j < end_row; ++j) {
                for (int i = 0; i < W; ++i) {
                    
                    color pixel_color(0.0, 0.0, 0.0);
                    for (int s_j = 0 ; s_j < sqrt_spp ; s_j++){
                        for (int s_i = 0 ; s_i < sqrt_spp ; s_i++){
                            ray r = get_ray(i, j, s_i, s_j);
                            pixel_color += ray_color(r, max_depth, world);
                        }
                    }
                    // 平均
                    pixel_color *= pixel_sample_scale;
                    // 存到 framebuffer
                    framebuffer[j * W + i] = pixel_color;
                    
                }
                bars[tid].tick();
     
            }

        };
        
        std::vector<std::thread> threads;
        threads.reserve(n_threads);
        int row_start = 0;
        
        for (unsigned t = 0; t < n_threads; ++t) {
            int row_end = (t == n_threads - 1) ? H : row_start + rows_per_thread;
            threads.emplace_back(worker, row_start, row_end, t);
            row_start = row_end;
        }

        for (auto& th : threads) th.join();

        std::ofstream ofs("out/img.ppm");
        ofs << "P3\n" << W << ' ' << H << "\n255\n";

        for (int idx = 0; idx < W * H; ++idx) {
            write_color(ofs, framebuffer[idx]);
        }
        std::clog << "Done.\n\n";
    }


    void render(const hittable& world){
        initialize();
        std::ofstream ofs("out/img.ppm");
        ofs << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color (0.0, 0.0, 0.0);
                for (int s_j = 0 ; s_j < sqrt_spp ; s_j++){
                    for (int s_i = 0 ; s_i < sqrt_spp ; s_i++){
                        ray r = get_ray(i, j, s_i, s_j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                }
                write_color(ofs, pixel_sample_scale * pixel_color);
            }
        }
        ofs.close();
    }

  private :
    int image_height;            // Rendered image height
    double pixel_sample_scale;   // Color scale factor for a sum of pixel samples
    int sqrt_spp;               // Square root of number of samples per pixel
    double recip_sqrt_spp;      // 1 / sqrt_spp
    point3 center;               // Camera center
    point3 pixel00_loc;          // Location of pixel 0, 0
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

        sqrt_spp = int(std::sqrt(samples_per_pixel));
        pixel_sample_scale = 1.0 / (sqrt_spp * sqrt_spp);
        recip_sqrt_spp = 1.0 / sqrt_spp;

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
    
    ray get_ray(int i, int j, int s_i, int s_j) const{
 
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j for stratified sample square s_i, s_j.
        // vec3 offset = sample_square();
        vec3 offset = sample_square_stratified(s_i, s_j);
        point3 pixel_sample =   pixel00_loc 
                                + ((i + offset.x()) * pixel_delta_u) 
                                + ((j + offset.y()) * pixel_delta_v);

        point3 ray_origin = defocus_angle < 0 ? center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;
        
        double ray_time = random_double();
        
        return ray(ray_origin, ray_direction, ray_time);

    }

    vec3 sample_square_stratified(int s_i, int s_j) const {
        // Returns the vector to a random point in the square sub-pixel specified by grid
        // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

        auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

        return vec3(px, py, 0);
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
        if (!world.hit(r, interval(0.001, infinity), rec))  //用bvh優化，原本對整體物件進行線性搜索O(n) -> O(log n)
            return background;
        
        ray scattered;
        color attenuation;
        color color_from_emmission = rec.mat->emitted(rec.u, rec.v, rec.p);
        
        if (!rec.mat->scatter(r, rec, attenuation, scattered))
            return color_from_emmission;
        
        color color_from_scatter = attenuation * ray_color(scattered, depth-1 , world);
        
        return color_from_scatter + color_from_emmission;

    }
};

#endif