/* pixelfilter: contains pixelfilter routines */

#ifndef _PIXELFILTER_H_
#define _PIXELFILTER_H_

class pixelFilter {
public:
  pixelFilter(void);
  virtual ~pixelFilter(void);
  virtual void sample(double *xi1, double *xi2);
};

class BoxFilter: public pixelFilter {
public:
  BoxFilter(void);
  ~BoxFilter(void);
  void sample(double *, double *);
};

class TentFilter: public pixelFilter {
public:
  TentFilter(void);
  ~TentFilter(void);
  void sample(double *, double *);
};

class NormalFilter: public pixelFilter {
public:
  NormalFilter(double s = .70710678, double d = 2.0);
  ~NormalFilter(void);
  void sample(double *, double *);

  double sigma;
  double dist;
};

#endif /* _PIXELFILTER_H_ */
