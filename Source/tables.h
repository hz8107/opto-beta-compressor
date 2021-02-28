#pragma once

#define TABLESIZE 1024

#define TANHSIZE 4096

extern const float Ttanh[TANHSIZE];

extern const float TRatio[TABLESIZE];

extern const float TinvRC[TABLESIZE];

extern const float TableMax;

#define TAP_NUM 32

extern const float fir_h[TAP_NUM];