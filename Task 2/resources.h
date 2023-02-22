#ifndef A2_RESOURCES_H
#define A2_RESOURCES_H
#include "pthread.h"
#include "stdint.h"
#include "stdlib.h"

/* STUDENT TODO:
 *  make sure shared resources are locked correctly
 *  add any needed variables here
 */

typedef enum{
  WASHING_OFF, 
  WASHING_ON,
  NEEDS_MAINTENANCE,
  CLOSED
} Mode;

typedef struct {
  pthread_t customer;
  sem_t customer_ready;
  bool washed_car_suc;
  pthread_mutex_t customer_lock;
  pthread_cond_t car_washed;
  ssize_t id;
} Customer;

typedef struct{
  pthread_t wash_bay;
  ssize_t id;
  Mode mode;

  pthread_mutex_t mode_lock;
  

  sem_t program_chosen;
  Customer* current_customer;


} WashBay;

typedef struct {
  pthread_t employee;
  ssize_t id;
} Employee;

#endif //A2_RESOURCES_H
