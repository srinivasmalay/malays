#ifndef NXTCAM_H
#define NXTCAM_H

#include <ev3dev.h>

#include <vector>
#include <string>

// Talks to an NXTcam device to perform image based tracking of 'blobs'.
class nxtcam {
public:
  // Description of a tracked blob.
  struct blob {
    int x1, y1;
    int x2, y2;
    int color;
  };
  // Type of a list of blobs.
  typedef std::vector<blob> blob_list;

  nxtcam(const ev3dev::port_type &port, int address = 0x01);
  ~nxtcam();
  
  // The address and port this device is connected to.
  int address() const { return address_; }
  const ev3dev::port_type &port() const  { return port_; }

  // Query information about the connected device.
  std::string version() const;
  std::string vendor_id() const;
  std::string device_id() const;

  // Command camera to begin tracking objects.
  void track_objects();
  // Command camera to begin tracking lines.
  void track_lines();
  // Command camera to stop tracking.
  void stop_tracking();
  
  // Get the currently tracked blobs from the camera.
  blob_list blobs() const;

protected:
  // NXTcam registers.
  enum cam_reg {
    reg_version = 0x00,
    reg_vendor_id = 0x08,
    reg_device_id = 0x10,
    reg_cmd = 0x41,
    reg_count = 0x42,
    reg_data = 0x43,
  };
  
  // Read size registers beginning at reg.
  void read(uint8_t reg, uint8_t *data, size_t size) const;
  // Write a single byte to a reg.
  void write(uint8_t reg, uint8_t data);

  // Send an ordered sequence of commands to the device.
  template <typename T, int N>
  void write_cmds(T (&commands)[N]) {
    for (int i = 0; i < N; i++) {
      write(reg_cmd, commands[i]);
    }
  }

  // Read N registers beginning at reg.
  template <typename T, int N>
  void read(uint8_t reg, T (&data)[N]) const {
    read(reg, reinterpret_cast<uint8_t*>(&data[0]), N*sizeof(T));
  }

  int fd_;
  int address_;
  ev3dev::port_type port_;
};

#endif