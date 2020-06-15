#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <FreeRTOS.h>
#include <task.h>

#include "application_freertos_prio.h"
#include "uart.h"
#include "adc.h"
#include "motor_ctrl.h"
#include "motor_encoder.h"

/////////////////////////////////////////////////////////////

static void init_clock(void)
{
   rcc_clock_setup_in_hse_8mhz_out_72mhz();

   // clock for GPIO port C: USER_LED
   rcc_periph_clock_enable(RCC_GPIOC);
}

/////////////////////////////////////////////////////////////

static void init_gpio(void)
{
   // USER_LED: PC13
   gpio_set_mode(GPIOC,
      GPIO_MODE_OUTPUT_2_MHZ,
      GPIO_CNF_OUTPUT_PUSHPULL,
      GPIO13);
}

/////////////////////////////////////////////////////////////

static void task_led(__attribute__((unused))void * pvParameters)
{
   while (1)
   {
      gpio_toggle(GPIOC, GPIO13);
      vTaskDelay(pdMS_TO_TICKS(500));
   }
}

/////////////////////////////////////////////////////////////

static void test_init(void)
{
   adc_init();
   motor_ctrl_init();
   motor_encoder_init();
}

/////////////////////////////////////////////////////////////

static void test_read_adc(void)
{
   uint16_t adc_val;

   adc_val = adc_get_value();

   printf("ADC : %04u - 0x%03x - %04umV\n",
          adc_val, adc_val, (adc_val * ADC_REF_VOLTAGE) / ADC_MAX_VALUE);
}

/////////////////////////////////////////////////////////////

static void test_set_pwm_duty(void)
{
   char input_buf[16];
   unsigned duty_value;

   printf("Enter duty value[0-65535]: ");
   fflush(stdout);

   fgets(input_buf, 16, stdin);
   sscanf(input_buf, "%u", &duty_value);

   motor_ctrl_pwm_set_duty(duty_value);
}

/////////////////////////////////////////////////////////////

static void test_set_direction(bool forward)
{
   motor_ctrl_pwm_set_direction(forward);
}

/////////////////////////////////////////////////////////////

static void test_brake(void)
{
   motor_ctrl_brake();
}

/////////////////////////////////////////////////////////////

static void test_zero_encoder(void)
{
   motor_encoder_zero_gearbox_shaft_pos();
}

/////////////////////////////////////////////////////////////

static void test_get_encoder(void)
{
   uint32_t pos = motor_encoder_get_gearbox_shaft_pos();
   uint32_t deg = (pos * 360) / MOTOR_ENCODER_CPR_GEAR_SHAFT;

   printf("POS : %04lu - 0x%03lx - DEG : %03lu\n",
          pos, pos, deg);
}

/////////////////////////////////////////////////////////////

static void print_test_menu(void)
{
  printf("\n");
  printf("heap-free: %u\n", xPortGetFreeHeapSize());
  printf("-----------------------------------\n");
  printf("--          TEST MENU            --\n");
  printf("-----------------------------------\n");
  printf(" 1. init\n");
  printf(" 2. read adc\n");
  printf(" 3. set pwm duty\n");
  printf(" 4. set direction - forward\n");
  printf(" 5. set direction - reverse\n");
  printf(" 6. brake\n");
  printf(" 7. zero encoder\n");
  printf(" 8. get encoder\n");
  printf("\n");
}

////////////////////////////////////////////////////////////

static void task_test(__attribute__((unused))void * pvParameters)
{
   char input_buf[16];
   int value;

   while (1)
   {
      print_test_menu();

      printf("Enter choice : ");
      fflush(stdout);

      fgets(input_buf, 16, stdin);
      sscanf(input_buf, "%d", &value);

      switch (value)
      {
         case 1:
            test_init();
            break;
         case 2:
            test_read_adc();
            break;
         case 3:
            test_set_pwm_duty();
            break;
         case 4:
            test_set_direction(true);
            break;
         case 5:
            test_set_direction(false);
            break;
         case 6:
            test_brake();
            break;
         case 7:
            test_zero_encoder();
            break;
         case 8:
            test_get_encoder();
            break;
         default:
            printf("*** Illegal choice : %s\n", input_buf);
      }
   }
}

/////////////////////////////////////////////////////////////

int main(void)
{
   // initialize hardware
   init_clock();
   init_gpio();
   uart_init();

   printf("\nirdecode_freertos - started\n");

   // Minium stack size for LED is 50 words (checked with configCHECK_FOR_STACK_OVERFLOW)
   xTaskCreate(task_led,  "LED",   50, NULL, TASK_LED_PRIO,  NULL);
   xTaskCreate(task_test, "TEST", 300, NULL, TASK_TEST_PRIO, NULL);

   printf("heap-free: %u\n", xPortGetFreeHeapSize());

   vTaskStartScheduler();
   while (1)
   {
      ;
   }

   return 0;
}

/////////////////////////////////////////////////////////////

#if (configASSERT_DEFINED == 1)

void vAssertCalled(unsigned long ulLine, const char * const pcFileName)
{
   taskENTER_CRITICAL();
   {
      printf("*** ASSERT => %s:%lu\n", pcFileName, ulLine);
      fflush(stdout);
   }
   taskEXIT_CRITICAL();

   while (1)
   {
      ;
   }
}
#endif // configASSERT_DEFINED

/////////////////////////////////////////////////////////////

#if (configCHECK_FOR_STACK_OVERFLOW == 1 || configCHECK_FOR_STACK_OVERFLOW == 2)

// Used while developing.
// FreeRTOS will call this function when detecting a stack overflow.
// The parameters could themselves be corrupted, in which case the
// pxCurrentTCB variable can be inspected directly
// Set a breakpoint in this function using the hw-debugger.

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   signed char *pcTaskName);

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   signed char *pcTaskName)
{
   (void) xTask;
   (void) pcTaskName;

   gpio_set(GPIOC, GPIO13);
   while (1)
   {
      ;
   }
}

#endif // configCHECK_FOR_STACK_OVERFLOW
