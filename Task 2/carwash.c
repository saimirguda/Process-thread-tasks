#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

#include "definitions.h"
#include "resources.h"
#include "vector.h"
/*
 * HINTS:
    ~  Make sure EVERY shared resource is locked properly!
    ~  Consider establishing a valid customer - automatic wash machine sync first
    ~  Be aware of 'lost' signals!
    ~
 */

int vacuum_stations[NUM_VACUUM_STATIONS];
vector free_wash_bays;
int count_washed_cars = 0;
bool carwash_opened = false;

/*
 * STUDENT TODO BEGIN:
 *  locking-related global variables may be declared here
 */
sem_t vacum_lock;
sem_t washbay_capacity_lock;

pthread_mutex_t vacum_lock_1;
pthread_mutex_t free_bays_lock;
pthread_mutex_t washing_lock ;
pthread_mutex_t cars_washed_locks ;

pthread_cond_t cw_opening;

/*
 * STUDENT TODO END:
 */

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure : 
    ~ shared resources are locked correctly
    ~ every customer gets to the vacuum station
 */
void goToTheVacuumStation(Customer* customer) 
{
  printf("Customer %zd goes to the vacuum station...\n", customer->id);
  int i = 0;
  sem_wait(&vacum_lock);
  pthread_mutex_lock(&vacum_lock_1);
  while (vacuum_stations[i] != 0) 
  {
    i++;
  }
  vacuum_stations[i] = 1;
  pthread_mutex_unlock(&vacum_lock_1);
  printf("Customer %zd vacuums his car...\n", customer->id);

  carVacuuming();

  pthread_mutex_lock(&vacum_lock_1);
  vacuum_stations[i] = 0;
  pthread_mutex_unlock(&vacum_lock_1);
  sem_post(&vacum_lock);
  printf("Customer %zd left vacuum station...\n", customer->id);
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure : 
    ~ shared resources are locked correctly
    ~ customers start looking for a free bay after opening
    ~ the customer gets a free, working wash bay
    ~ be aware of "lost signals"
 */
WashBay* driveIntoTheWashBay(Customer* customer) 
{

  pthread_mutex_lock(&washing_lock);
  while(carwash_opened == false)
    pthread_cond_wait(&cw_opening, &washing_lock);
  pthread_mutex_unlock(&washing_lock);
  printf("Customer %zd is looking for a free wash bay.\n", customer->id);

  WashBay* free_wash_bay = 0;
  sem_wait(&washbay_capacity_lock);
  pthread_mutex_lock(&free_bays_lock);
  // find a free car wash bay
  vector_iterator it = vector_begin(&free_wash_bays);
  vector_iterator it_end = vector_end(&free_wash_bays);
  vector_iterator wash_bay_it;

  for (; it != it_end; ++it) 
  {
    WashBay* tmp = *it;
    pthread_mutex_lock(&tmp->mode_lock);
    if (tmp->mode != NEEDS_MAINTENANCE) 
    {
      wash_bay_it = it;
      pthread_mutex_unlock(&tmp->mode_lock);
      break;
    }
    pthread_mutex_unlock(&tmp->mode_lock);
  }

  driveIn();

  free_wash_bay = *wash_bay_it;

  if (free_wash_bay) 
  {
    vector_erase(&free_wash_bays, wash_bay_it);
    printf("Customer %zd found a free wash bay %zd\n", customer->id, free_wash_bay->id);
  }
  pthread_mutex_unlock(&free_bays_lock);
  //pthread_mutex_lock(&free_wash_bay->customer_lock);
  if(free_wash_bay && free_wash_bay->current_customer)
  {
    printf("Customer %zd tries to drive into a used wash bay :o\n", customer->id);
  }
  free_wash_bay->current_customer = customer;
  //pthread_mutex_unlock(&free_wash_bay->customer_lock);

  return free_wash_bay;
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure : 
    ~ shared resources are locked correctly
    ~ the customer picks up the car at the end of the program
    ~ auto cleaning is triggered upon leaving the wash bay
 */
void leaveTheWashBay(Customer* customer, WashBay* wash_bay) 
{
  pthread_mutex_lock(&customer->customer_lock);
  while(customer->washed_car_suc == false)
    pthread_cond_wait(&customer->car_washed, &customer->customer_lock);

  if(wash_bay->current_customer != customer)
  {
    printf("Customer %zd didn't find his car in the wash bay :o.\n", customer->id);
  }

  sem_post(&customer->customer_ready);
  
  pthread_mutex_unlock(&customer->customer_lock);

  //pthread_mutex_lock(&wash_bay->mode_lock);
  if(isMaintenanceNeeded())
  {
    pthread_mutex_lock(&wash_bay->mode_lock);
    wash_bay->mode = NEEDS_MAINTENANCE;
    pthread_mutex_unlock(&wash_bay->mode_lock);
  }
  else
  {
    pthread_mutex_lock(&wash_bay->mode_lock);
    wash_bay->mode = WASHING_OFF;
    pthread_mutex_unlock(&wash_bay->mode_lock);
  }
  
  //pthread_mutex_unlock(&wash_bay->mode_lock);
  printf("Customer %zd leaves the wash bay...\n", customer->id);

}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure :
    ~ shared resources are locked correctly
    ~ the program selection starts the washing process
 */
void selectAWashingProgram(WashBay* wash_bay, Customer* customer)
{
  pthread_mutex_lock(&wash_bay->mode_lock);
  wash_bay->mode = WASHING_ON;
  sem_post(&wash_bay->program_chosen);
  pthread_mutex_unlock(&wash_bay->mode_lock);
  printf("Customer %zd selected a washing program and can have a coffee break now.\n", customer->id);
}

// ------------------------------------------------------------------------
void* takeABreakAtCarWashPark(Customer* customer) 
{
  WashBay* wash_bay = driveIntoTheWashBay(customer);
  selectAWashingProgram(wash_bay, customer);
  coffeeTime();
  leaveTheWashBay(customer, wash_bay);
  goToTheVacuumStation(customer);
  
  return NULL;
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure shared resources are locked correctly
 */
void maintainingWashBay(WashBay* wash_bay) 
{
  washBayMaintenance();
  pthread_mutex_lock(&wash_bay->mode_lock);
  wash_bay->mode = WASHING_OFF;
  pthread_mutex_unlock(&wash_bay->mode_lock);
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure 
    ~ shared resources are locked correctly
    ~ employees don't start working before car wash park is opened
 */
void* checkWashBays(Employee* employee) 
{
  pthread_mutex_lock(&washing_lock);
  while(carwash_opened == false)
    pthread_cond_wait(&cw_opening, &washing_lock);
  pthread_mutex_unlock(&washing_lock);
  while (1) 
  {
    printf("Employee %zd is checking wash bays...\n", employee->id);

    WashBay* wash_bay = 0;
    pthread_mutex_lock(&free_bays_lock);
    vector_iterator it = vector_begin(&free_wash_bays);

    for (; it != vector_end(&free_wash_bays); ++it) 
    {
      WashBay* tmp = *it;
      pthread_mutex_lock(&tmp->mode_lock);
      if (tmp->mode == NEEDS_MAINTENANCE) 
      {
        pthread_mutex_unlock(&tmp->mode_lock);
        wash_bay = *it;
        vector_erase(&free_wash_bays, it);
        break;
      }
      pthread_mutex_unlock(&tmp->mode_lock);
    }
    pthread_mutex_unlock(&free_bays_lock);
    if (!wash_bay) 
    {
      printf("Employee %zd has nothing to do...\n", employee->id);
      coffeeTime();
      continue;
    }
  
    maintainingWashBay(wash_bay);
    pthread_mutex_lock(&free_bays_lock);
    vector_push_back(&free_wash_bays, wash_bay);
    sem_post(&washbay_capacity_lock);
    pthread_mutex_unlock(&free_bays_lock);
  }
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure :
    ~ shared resources are locked correctly
    ~ automatic wash machines are not active if the car wash park is closed
    ~ washing process doesn't start before program selection <-- mode correspondingly changed
    ~ the customer is notified as soon as the washing process is finished
    ~ auto cleaning activated afterward
 */
void automaticWashing(WashBay* wash_bay) 
{
  pthread_mutex_lock(&washing_lock);
  while(carwash_opened == false)
    pthread_cond_wait(&cw_opening, &washing_lock);
  pthread_mutex_unlock(&washing_lock);
  while(1)
  {
    printf("WashBay %zd : waiting for new customers.\n", wash_bay->id);
    sem_wait(&wash_bay->program_chosen);
    pthread_mutex_lock(&wash_bay->mode_lock);
    if(wash_bay->mode == CLOSED)
    {
      pthread_mutex_unlock(&wash_bay->mode_lock);
      break;
    }

    if(wash_bay->mode != WASHING_ON)
    {
      printf("WashBay %zd : washing before wash program selected.\n", wash_bay->id); 
    }
    pthread_mutex_unlock(&wash_bay->mode_lock);
    printf("WashBay %zd : washing car of customer %zd.\n", 
            wash_bay->id, wash_bay->current_customer->id);

    washTheCar(); 

    printf("WashBay %zd : the car of customer %zd washed, can be picked up.\n", 
            wash_bay->id, wash_bay->current_customer->id);
    

    pthread_mutex_lock(&wash_bay->current_customer->customer_lock);
    wash_bay->current_customer->washed_car_suc = true;
    pthread_cond_signal(&wash_bay->current_customer->car_washed);
    pthread_mutex_unlock(&wash_bay->current_customer->customer_lock);

    sem_wait(&wash_bay->current_customer->customer_ready);

    wash_bay->current_customer = 0;


    automizedCleaning();
    pthread_mutex_lock(&free_bays_lock);
    vector_push_back(&free_wash_bays, wash_bay);
    pthread_mutex_unlock(&free_bays_lock);

    pthread_mutex_lock(&wash_bay->mode_lock);
    if(wash_bay->mode != NEEDS_MAINTENANCE)
    {
      printf("WashBay %zd is ready for new customers.\n", wash_bay->id);
      sem_post(&washbay_capacity_lock);
    }
    pthread_mutex_unlock(&wash_bay->mode_lock);


    pthread_mutex_lock(&cars_washed_locks);
    count_washed_cars++;
    pthread_mutex_unlock(&cars_washed_locks);
  }
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure : 
    ~ shared resources are locked correctly
    ~ everyone is notified about the opening
 */
void openCarWashPark()
{
  pthread_mutex_lock(&washing_lock);
  carwash_opened = true;
  pthread_mutex_unlock(&washing_lock);
  printf("CAR WASH PARK OPENED!\n");
  pthread_cond_broadcast(&cw_opening);
}

// ------------------------------------------------------------------------
/* STUDENT TODO:
 *  make sure :
    ~ shared resources are locked correctly
    ~ mutex/cond/sem initialized and destroyed accordingly
 */
int main(int argc, char* argv[]) 
{
  srand(time(NULL));

  if (argc != 4) 
  {
    fprintf(stderr, "Usage: %s <num_employees> <num_customers> <num_wash_bays>\n", argv[0]);
    exit(-1);
  }

  // check the validity of parameters
  ssize_t num_working_staff = atoi(argv[1]);
  ssize_t num_customers = atoi(argv[2]);
  ssize_t num_wash_bays = atoi(argv[3]);

  checkParametersValidity(num_working_staff, num_customers, num_wash_bays);
  
  Employee* employees = malloc(num_working_staff * sizeof(Employee));
  Customer* customers = malloc(num_customers * sizeof(Customer));
  
  if (!employees || !customers) 
  {
    free(employees);
    free(customers);
    fprintf(stderr, "Could not allocate memory!\n");
    exit(-1);
  }
  
  // handle free_wash_bays vector
  vector_init(&free_wash_bays);
  WashBay* wash_bays[num_wash_bays];

  sem_init(&washbay_capacity_lock, 0 , num_wash_bays);
  sem_init(&vacum_lock, 0 , NUM_VACUUM_STATIONS);

  pthread_mutex_init(&vacum_lock_1, 0);
  pthread_mutex_init(&free_bays_lock, 0);
  pthread_mutex_init(&washing_lock, 0);
  pthread_mutex_init(&cars_washed_locks, 0);

  pthread_cond_init(&cw_opening, 0);
  for (ssize_t i = 0; i < num_wash_bays; i++) 
  {
    WashBay* wash_bay = malloc(sizeof(WashBay));

    if(!wash_bay)
    {
      free(wash_bay);
      fprintf(stderr, "Could not allocate memory!\n");
      exit(-1);
    }

    wash_bay->mode = WASHING_OFF;
    wash_bay->id = i;
    wash_bay->current_customer = 0;
    pthread_mutex_init(&wash_bay->mode_lock, 0);
    
    sem_init(&wash_bay->program_chosen, 0,0);
    vector_push_back(&free_wash_bays, wash_bay);
    

    // create car wash machine threads
    wash_bays[i] = wash_bay;
    assert(!pthread_create(&wash_bay->wash_bay, NULL, 
                           (void*(*)(void*))automaticWashing, (void*)wash_bay));   
    
  }

  // create employee threads
  for (ssize_t i = 0; i < num_working_staff; i++) 
  {
    employees[i].id = i;
    assert(!pthread_create(&employees[i].employee, NULL, 
                      (void*(*)(void*))checkWashBays, (void*)&employees[i]));
  }

  // create customer threads
  for (ssize_t i = 0; i < num_customers; i++) 
  {
    customers[i].id = i;
    sem_init(&customers[i].customer_ready, 0, 0);
    pthread_mutex_init(&customers[i].customer_lock, 0);
    pthread_cond_init(&customers[i].car_washed, 0);
    pthread_mutex_init(&customers[i].customer_lock, 0);
    customers[i].washed_car_suc = false;
    assert(!pthread_create(&customers[i].customer, NULL, 
                  (void*(*)(void*))takeABreakAtCarWashPark, (void*)&customers[i]));
  }
    
  // create a mystic employee to open the car wash park
  pthread_t mystic_employee;
  assert(!pthread_create(&mystic_employee, NULL, 
              (void*(*)(void*))&openCarWashPark, NULL));
  
  for (ssize_t i = 0; i < num_customers; i++) 
  {
    pthread_join(customers[i].customer, NULL);
  }

  for (ssize_t i = 0; i < num_working_staff; i++) 
  {
    pthread_cancel(employees[i].employee);
    pthread_join(employees[i].employee, NULL);
  }

  for(size_t i = 0; i < num_wash_bays; ++i)
  {
    pthread_mutex_lock(&wash_bays[i]->mode_lock);
    wash_bays[i]->mode = CLOSED;
    sem_post(&wash_bays[i]->program_chosen);
    pthread_mutex_unlock(&wash_bays[i]->mode_lock);
    pthread_join(wash_bays[i]->wash_bay, NULL);
  }

  pthread_join(mystic_employee, NULL);

  printf("CAR WASH PARK CLOSED!\n");
  sem_destroy(&vacum_lock);
  sem_destroy(&washbay_capacity_lock);

  assert(!pthread_mutex_destroy(&vacum_lock_1));
  assert(!pthread_mutex_destroy(&free_bays_lock));
  assert(!pthread_mutex_destroy(&washing_lock));
  assert(!pthread_mutex_destroy(&cars_washed_locks));

  pthread_cond_destroy(&cw_opening);


  for (ssize_t i = 0; i < num_customers; i++) 
  {
    assert(!sem_destroy(&customers->customer_ready));
    assert(!pthread_cond_destroy(&customers->car_washed));

  }
  free(employees);
  free(customers);
  
  
  vector_iterator it = vector_begin(&free_wash_bays);
  while (it != vector_end(&free_wash_bays)) 
  {
    WashBay * tmp = *it;
    assert(!pthread_mutex_destroy(&tmp->mode_lock));
    assert(!sem_destroy(&tmp->program_chosen));
    
    free(*it);
    vector_erase(&free_wash_bays, it);
  }
  vector_destroy(&free_wash_bays);

  return 0;
}
