#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <signal.h>
#include <iostream>

#include "pid_motor.h"

namespace {
  
std::vector<pid_motor *> live_motors;
std::mutex motors_lock;

std::thread controller_thread;
void controller_main() {
  // Clock to use for controllers.
  typedef std::chrono::high_resolution_clock clock;

  // Sampling period for the controllers.
  static const std::chrono::milliseconds dt(10);

  auto t0 = clock::now();
  for(auto t = t0; ; t += dt) {
    if (dt.count() > 0) {
      std::lock_guard<std::mutex> lock(motors_lock);
      for (auto i : live_motors) {
        i->tick(dt.count());
      }
      if (live_motors.empty())
        break;
    }
    std::this_thread::sleep_until(t);
  }
}

}

pid_motor::pid_motor(const ev3dev::port_type &port) : m_(port), pid_(10000, 10000, 0) {
  reset();
  
  {
    std::lock_guard<std::mutex> lock(motors_lock);
    live_motors.push_back(this);
  }

  // If we don't have a controller thread, make one.
  if (!controller_thread.joinable()) {
    std::thread new_thread(controller_main);
    std::swap(controller_thread, new_thread);
  }
}

pid_motor::~pid_motor() {
  motors_lock.lock();
  live_motors.erase(std::find(live_motors.begin(), live_motors.end(), this));

  // If there are no more live motors, wait for the controller thread to complete.
  if (live_motors.empty()) {
    motors_lock.unlock();
    controller_thread.join();
  } else {
    motors_lock.unlock();
  }

  m_.reset();
}

void pid_motor::run() {
  {
    std::lock_guard<std::mutex> lock(this->lock_);
    pid_.reset();
  }
  m_.set_run_mode(ev3dev::motor::run_mode_forever);
  tick(1);
  m_.run();
}

void pid_motor::stop() {
  std::lock_guard<std::mutex> lock(this->lock_);
  pid_.reset();
  m_.stop();
}

void pid_motor::reset() {
  std::lock_guard<std::mutex> lock(this->lock_);
  pid_.reset();
  m_.reset();
  max_duty_cycle_ = 100;
}

void pid_motor::tick(int dt) {
  std::lock_guard<std::mutex> lock(this->lock_);
  pid_.tick(dt, position());
  int y = pid_.output()/1024;
  y = std::min(std::max(y, -max_duty_cycle_), max_duty_cycle_);
  m_.set_duty_cycle_setpoint(y);
}

std::tuple<int, int, int> pid_motor::K() const {
  // Controller thread doesn't write this values, so we don't need to lock our mutex.
  return std::tie(pid_.Kp, pid_.Ki, pid_.Kd);
}

void pid_motor::set_K(int Kp, int Ki, int Kd) {
  std::lock_guard<std::mutex> lock(this->lock_);
  pid_.Kp = Kp;
  pid_.Ki = Ki;
  pid_.Kd = Kd;
}

void pid_motor::set_max_duty_cycle(int x) {
  std::lock_guard<std::mutex> lock(this->lock_);
  max_duty_cycle_ = x;
}