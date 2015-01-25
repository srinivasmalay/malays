#include <fstream>

#include "arg_port.h"
#include "camera.h"
#include "vector2.h"
#include "vector3.h"

static cl::group stereo_group("Stereo camera parameters");

struct camera_config {
  arg_port port;
  cl::arg<vector2i> resolution;
  cl::arg<vector2f> distortion;
  cl::arg<matrix<float, 3, 3>> calibration;

  cl::arg<quaternionf> R;
  cl::arg<vector3f> x;

  camera_config(
      const std::string &prefix,
      const ev3::port_type &port) : 
  port(
    port,
    cl::name(prefix + "-port"),
    cl::desc("Input port for the camera."),
    stereo_group),
  resolution(
    vector2i(176, 144),
    cl::name(prefix + "-resolution"),
    cl::desc("Resolution of the cameras, in pixels."),
    stereo_group),
  distortion(
    vector2f(0.0f, 0.0f),
    cl::name(prefix + "-distortion"),
    cl::desc("Radial distortion parameters of the camera."),
    stereo_group),
  calibration(
    matrix3x3f(1.0f),
    cl::name(prefix + "-calibration"),
    cl::desc("Camera calibration matrix."),
    stereo_group),
  R(
    quaternionf(1.0f, 0.0f, 0.0f, 0.0f),
    cl::name(prefix + "-orientation"),
    cl::desc("Camera orientation quaternion."),
    stereo_group),
  x(
    vector3f(0.0f),
    cl::name(prefix + "-origin"),
    cl::desc("Camera position."),
    stereo_group)
  {}

  cameraf to_camera() const {
    return cameraf(
        vector_cast<float>(*resolution),
        distortion,
        calibration,
        R,
        x);
  }
};

class stereo_config_file_arg : public cl::arg<std::string> {
public:
  stereo_config_file_arg() : cl::arg<std::string>(
      "stereo_config",
      cl::name("stereo-config")) {}

  void parse(std::list<const char *> &argv) {
    cl::arg<std::string>::parse(argv);
    
    std::ifstream file(*this);
    while (file) {
      std::string line;
      std::getline(file, line);
      
      std::string::size_type space = line.find(' ');
      if (space != std::string::npos) {
        line[space] = '\0';
        const char *args[] = {
          &line[0],
          &line[space + 1]
        };
        cl::parse(2, args);
      }
    }
  }
};

struct stereo_config {
  camera_config cam0{"cam0", ev3::INPUT_1};
  camera_config cam1{"cam1", ev3::INPUT_4};
  stereo_config_file_arg config_file;
  
  std::pair<cameraf, cameraf> cameras() const {
    return std::make_pair(cam0.to_camera(), cam1.to_camera());
  }
};