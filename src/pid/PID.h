#ifndef PID_H
#define PID_H

class DiscretePID {
public:
  DiscretePID(float setpoint, bool invert);
  float compute(float measurement);

private:
  float setpoint;
  float error_window[3];
  float u_window[3];
  float b1, b2, b3;
  float c1, c2, c3;
  float kpl, ka, kt;
  float pi;
  float taudm, tauim, taufm;
  float a1, a2, a3, a4, a5, a6, a7, a8;
  float Mp_raw, Mr_raw, Mp, Mr, k, itot;
  float Me, Jm, ksl, beta, kv, kta;
  float Wr, War, Wb; 
  float Ts;
  bool invertOutput;
};

#endif
