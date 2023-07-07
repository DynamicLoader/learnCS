#ifndef TNAM_H
#define TNAM_H

#include "rtwtypes.h"
#include "coder_array.h"
#include "omp.h"
#include <cstddef>
#include <cstdlib>
#include <atomic>

class TNAM
{
  public:
    TNAM();
    ~TNAM();
    void triangular_decode(double size, const short Q_tri[327675], const short Q[262144],
                           unsigned short num_of_total_triangular, unsigned short point_num, unsigned short line_num,
                           coder::array<boolean_T, 2U> &res);
    void triangular_raster(double size, const coder::array<boolean_T, 2U> &f, unsigned short *point_num,
                           unsigned short *line_num, unsigned short *num_of_total_triangular, short Q_tri[327675],
                           short Q[262144]);

    uint16_t getEncodeProgress()
    {
      return _encProgress;
    }

    uint16_t getDecodeProgress()
    {
      return _decProgress;
    }

  private:
    std::atomic_uint16_t _encProgress = 0;
    std::atomic_uint16_t _decProgress = 0;
};

#endif
