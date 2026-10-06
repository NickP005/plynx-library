/*
 * SimpleTimer.h
 *
 * SimpleTimer - A timer library for Arduino.
 * Author: mromani@ottotecnica.com
 * Copyright (c) 2010 OTTOTECNICA Italy
 *
 * Modifications by Bill Knight <billk@rosw.com> 18March2017
 *
 * This library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser
 * General Public License along with this library; if not,
 * write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

/**
 * @file       PlynxTimer.h
 * @brief      PlynxTimer, which runs functions later or at regular intervals.
 */

#ifndef PLYNXTIMER_H
#define PLYNXTIMER_H

#include <Plynx/PlynxDebug.h>

// Replace SimpleTimer
#define SIMPLETIMER_H
#define SimpleTimer PlynxTimer

/** @brief A function with no parameters that a PlynxTimer calls. */
typedef void (*timer_callback)(void);
/** @brief A function that a PlynxTimer calls with the pointer given to it. */
typedef void (*timer_callback_p)(void *);

/**
 * @brief Runs your functions every few seconds, or once after a wait, while the
 * board stays connected.
 *
 * Use it instead of delay(). delay() stops Plynx.run() too, and the board drops
 * off the app. One PlynxTimer holds up to 16 timers. Create it at the top of
 * the sketch, set the timers in setup(), call timer.run() in loop(). Compiler
 * errors call it SimpleTimer.
 *
 * @code
 * PlynxTimer timer;
 *
 * void sendUptime()
 * {
 *   Plynx.virtualWrite(V5, millis() / 1000);
 * }
 *
 * void setup()
 * {
 *   // Plynx.begin(...) as in the example for your board
 *   timer.setInterval(1000L, sendUptime);   // every second
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 *   timer.run();
 * }
 * @endcode
 *
 * @warning Without timer.run() in loop() no timer ever fires.
 *
 * @warning Your function runs inside timer.run(). A delay() in it stops
 * Plynx.run() just the same.
 *
 * @see setInterval, setTimeout, PlynxProtocol::run
 */
class SimpleTimer {

public:
    /** @brief How many timers one PlynxTimer holds, 16. */
    const static int MAX_TIMERS = 16;

    /** @brief For setTimer(), a timer that repeats forever. */
    const static int RUN_FOREVER = 0;
    /** @brief For setTimer(), a timer that runs once. */
    const static int RUN_ONCE = 1;

    /**
     * @brief Creates an empty timer object.
     *
     * @code
     * PlynxTimer timer;   // at the top level of the sketch
     * @endcode
     */
    SimpleTimer();

    /**
     * @brief Clears all the timers. The first setInterval(), setTimeout() or
     * setTimer() calls it by itself.
     *
     * @code
     * PlynxTimer timer;
     *
     * void forgetEverything()
     * {
     *   timer.init();
     * }
     * @endcode
     *
     * @see deleteTimer
     */
    void init();

    /**
     * @brief Calls the functions whose time has come. Put it in loop(), next to
     * Plynx.run().
     *
     * A timer late by several intervals runs once and skips the missed runs.
     *
     * @code
     * PlynxTimer timer;
     *
     * void loop()
     * {
     *   Plynx.run();
     *   timer.run();
     * }
     * @endcode
     *
     * @see setInterval
     */
    void run();

    /**
     * @brief Calls a function every `d` milliseconds, forever. Set it once in
     * setup().
     *
     * @param d  The interval in milliseconds. On AVR boards write 60 * 1000L,
     *           because 60 * 1000 overflows an `int` there.
     * @param f  The function to call, without parentheses. It takes no
     *           parameters and returns nothing.
     * @return The number of the timer, from 0 to 15, for deleteTimer() and the
     *         others. -1 when all 16 are taken.
     *
     * @code
     * PlynxTimer timer;
     *
     * void sendLight()
     * {
     *   Plynx.virtualWrite(V2, analogRead(A0));
     * }
     *
     * void setup()
     * {
     *   // Plynx.begin(...) as in the example for your board
     *   timer.setInterval(2000L, sendLight);
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     *   timer.run();
     * }
     * @endcode
     *
     * @warning A virtualWrite() every few milliseconds floods the connection. 1 to
     * 10 sends per second is plenty for the app.
     *
     * @warning In loop() it adds a new timer at every turn, and after 16 turns
     * there is no room left.
     *
     * @see setTimeout, setTimer, deleteTimer, changeInterval
     */
    int setInterval(unsigned long d, timer_callback f);

    /**
     * @brief Calls a function every `d` milliseconds, forever, passing it a
     * pointer.
     *
     * The same as the other setInterval(), for one function that serves
     * several things, each with its own data.
     *
     * @param d  The interval in milliseconds.
     * @param f  The function to call, as `void name(void* p)`.
     * @param p  The pointer the function receives.
     * @return The number of the timer, or -1 when all 16 are taken.
     *
     * @code
     * PlynxTimer timer;
     * int ledA = 2;
     * int ledB = 4;
     *
     * void blink(void* p)
     * {
     *   int pin = *(int*)p;
     *   digitalWrite(pin, !digitalRead(pin));
     * }
     *
     * void setup()
     * {
     *   timer.setInterval(500L, blink, &ledA);
     *   timer.setInterval(800L, blink, &ledB);
     * }
     * @endcode
     *
     * @see setInterval
     */
    int setInterval(unsigned long d, timer_callback_p f, void* p);

    /**
     * @brief Calls a function once, `d` milliseconds from now. For example, turns a
     * relay off a few seconds after the app turned it on.
     *
     * The timer frees its place after the call.
     *
     * @param d  The wait in milliseconds.
     * @param f  The function to call, with no parameters.
     * @return The number of the timer, or -1 when all 16 are taken.
     *
     * @code
     * PlynxTimer timer;
     *
     * void relayOff()
     * {
     *   digitalWrite(4, LOW);
     * }
     *
     * PLYNX_WRITE(V1)
     * {
     *   digitalWrite(4, HIGH);
     *   timer.setTimeout(3000L, relayOff);   // off again in 3 seconds
     * }
     * @endcode
     *
     * @see setInterval, setTimer
     */
    int setTimeout(unsigned long d, timer_callback f);

    /**
     * @brief Calls a function once, `d` milliseconds from now, passing it a
     * pointer.
     *
     * @param d  The wait in milliseconds.
     * @param f  The function to call, as `void name(void* p)`.
     * @param p  The pointer the function receives.
     * @return The number of the timer, or -1 when all 16 are taken.
     *
     * @code
     * PlynxTimer timer;
     * int valve = 5;
     *
     * void closeValve(void* p)
     * {
     *   digitalWrite(*(int*)p, LOW);
     * }
     *
     * void water()
     * {
     *   digitalWrite(valve, HIGH);
     *   timer.setTimeout(10000L, closeValve, &valve);
     * }
     * @endcode
     *
     * @see setTimeout
     */
    int setTimeout(unsigned long d, timer_callback_p f, void* p);

    /**
     * @brief Calls a function every `d` milliseconds, `n` times, then stops.
     *
     * @param d  The interval in milliseconds.
     * @param f  The function to call, with no parameters.
     * @param n  How many times. RUN_FOREVER (0) repeats without end.
     * @return The number of the timer, or -1 when all 16 are taken.
     *
     * @code
     * PlynxTimer timer;
     *
     * void beep()
     * {
     *   tone(8, 1000, 100);
     * }
     *
     * void alarm()
     * {
     *   timer.setTimer(300L, beep, 5);   // five beeps
     * }
     * @endcode
     *
     * @see setInterval, setTimeout
     */
    int setTimer(unsigned long d, timer_callback f, unsigned n);

    /**
     * @brief Calls a function every `d` milliseconds, `n` times, passing it a
     * pointer.
     *
     * @param d  The interval in milliseconds.
     * @param f  The function to call, as `void name(void* p)`.
     * @param p  The pointer the function receives.
     * @param n  How many times.
     * @return The number of the timer, or -1 when all 16 are taken.
     *
     * @code
     * PlynxTimer timer;
     * int led = 2;
     *
     * void flip(void* p)
     * {
     *   int pin = *(int*)p;
     *   digitalWrite(pin, !digitalRead(pin));
     * }
     *
     * void signal()
     * {
     *   timer.setTimer(200L, flip, &led, 6);   // three blinks
     * }
     * @endcode
     *
     * @see setTimer
     */
    int setTimer(unsigned long d, timer_callback_p f, void* p, unsigned n);

    /**
     * @brief Changes the interval of a running timer, counted from now. For
     * example from a Slider in the app.
     *
     * @param numTimer  The number returned by setInterval() or the others.
     * @param d         The new interval in milliseconds.
     * @return true when the timer exists, false for a free place.
     *
     * @code
     * PlynxTimer timer;
     * int sendTimer;
     *
     * void sendReading()
     * {
     *   Plynx.virtualWrite(V5, analogRead(A0));
     * }
     *
     * void setup()
     * {
     *   sendTimer = timer.setInterval(1000L, sendReading);
     * }
     *
     * PLYNX_WRITE(V6)
     * {
     *   timer.changeInterval(sendTimer, param.asInt() * 1000L);   // seconds
     * }
     * @endcode
     *
     * @see setInterval, restartTimer
     */
    bool changeInterval(unsigned numTimer, unsigned long d);

    /**
     * @brief Removes a timer and frees its place.
     *
     * @param numTimer  The number returned by setInterval() or the others.
     *
     * @code
     * PlynxTimer timer;
     * int blinkTimer = -1;
     *
     * void blink()
     * {
     *   digitalWrite(2, !digitalRead(2));
     * }
     *
     * PLYNX_WRITE(V1)
     * {
     *   if (param.asInt() && blinkTimer < 0) {
     *     blinkTimer = timer.setInterval(500L, blink);
     *   } else if (!param.asInt() && blinkTimer >= 0) {
     *     timer.deleteTimer(blinkTimer);
     *     blinkTimer = -1;               // the number may go to a new timer
     *   }
     * }
     * @endcode
     *
     * @warning The next setInterval() can reuse the same number. Keep -1 in
     * your variable after deleting, or a second deleteTimer() removes another
     * timer.
     *
     * @see setInterval, disable
     */
    void deleteTimer(unsigned numTimer);

    /**
     * @brief Starts counting the interval of a timer again from now.
     *
     * @param numTimer  The number returned by setInterval() or the others.
     *
     * @code
     * PlynxTimer timer;
     * int idleTimer;
     *
     * void sleepScreen()
     * {
     *   digitalWrite(2, LOW);
     * }
     *
     * void setup()
     * {
     *   idleTimer = timer.setInterval(30000L, sleepScreen);
     * }
     *
     * PLYNX_WRITE(V1)
     * {
     *   timer.restartTimer(idleTimer);   // any touch keeps the screen on
     * }
     * @endcode
     *
     * @see changeInterval
     */
    void restartTimer(unsigned numTimer);

    /**
     * @brief Tells whether a timer is running or paused.
     *
     * @param numTimer  The number returned by setInterval() or the others.
     * @return true when the timer is running, false when it is paused or the
     *         place is free.
     *
     * @code
     * PlynxTimer timer;
     * int pumpTimer;
     *
     * void showPump()
     * {
     *   Plynx.virtualWrite(V7, timer.isEnabled(pumpTimer) ? 1 : 0);
     * }
     * @endcode
     *
     * @see enable, disable
     */
    bool isEnabled(unsigned numTimer);

    /**
     * @brief Resumes a paused timer.
     *
     * @param numTimer  The number returned by setInterval() or the others.
     *
     * @code
     * PlynxTimer timer;
     * int pumpTimer;
     *
     * PLYNX_WRITE(V7)
     * {
     *   if (param.asInt()) {
     *     timer.enable(pumpTimer);
     *   } else {
     *     timer.disable(pumpTimer);
     *   }
     * }
     * @endcode
     *
     * @see disable, toggle
     */
    void enable(unsigned numTimer);

    /**
     * @brief Pauses a timer without removing it.
     *
     * While paused the timer keeps counting and skips its calls, so after
     * enable() it fires on its usual schedule.
     *
     * @param numTimer  The number returned by setInterval() or the others.
     *
     * @code
     * PlynxTimer timer;
     * int reportTimer;
     *
     * PLYNX_DISCONNECTED()
     * {
     *   timer.disable(reportTimer);   // nothing to report while offline
     * }
     *
     * PLYNX_CONNECTED()
     * {
     *   timer.enable(reportTimer);
     * }
     * @endcode
     *
     * @see enable, deleteTimer
     */
    void disable(unsigned numTimer);

    /**
     * @brief Resumes the paused timers.
     *
     * It acts on the timers made with setInterval(), and on the ones made with
     * setTimeout() or setTimer() that have not fired yet.
     *
     * @code
     * PlynxTimer timer;
     *
     * PLYNX_CONNECTED()
     * {
     *   timer.enableAll();
     * }
     * @endcode
     *
     * @see disableAll
     */
    void enableAll();

    /**
     * @brief Pauses all the timers.
     *
     * It acts on the timers made with setInterval(), and on the ones made with
     * setTimeout() or setTimer() that have not fired yet.
     *
     * @code
     * PlynxTimer timer;
     *
     * PLYNX_DISCONNECTED()
     * {
     *   timer.disableAll();
     * }
     * @endcode
     *
     * @see enableAll
     */
    void disableAll();

    /**
     * @brief Pauses a running timer, or resumes a paused one.
     *
     * @param numTimer  The number returned by setInterval() or the others.
     *
     * @code
     * PlynxTimer timer;
     * int blinkTimer;
     *
     * PLYNX_WRITE(V1)
     * {
     *   if (param.asInt()) {          // on press, not on release
     *     timer.toggle(blinkTimer);
     *   }
     * }
     * @endcode
     *
     * @see enable, disable
     */
    void toggle(unsigned numTimer);

    /**
     * @brief Counts the timers in use.
     *
     * @return How many of the 16 places hold a timer.
     *
     * @code
     * PlynxTimer timer;
     *
     * void report()
     * {
     *   Serial.println(timer.getNumTimers());
     * }
     * @endcode
     *
     * @warning Before the first setInterval(), setTimeout() or setTimer() the
     * timer is not set up yet and the number is meaningless.
     *
     * @see getNumAvailableTimers
     */
    unsigned getNumTimers();

    /**
     * @brief Counts the free places for new timers.
     *
     * @return How many timers can still be added, out of 16.
     *
     * @code
     * PlynxTimer timer;
     *
     * void addIfRoom()
     * {
     *   if (timer.getNumAvailableTimers() > 0) {
     *     timer.setTimeout(1000L, addIfRoom);
     *   }
     * }
     * @endcode
     *
     * @warning Before the first setInterval(), setTimeout() or setTimer() it
     * says 17.
     *
     * @see getNumTimers
     */
    unsigned getNumAvailableTimers() { return MAX_TIMERS - numTimers; };

private:
    // deferred call constants
    const static int DEFCALL_DONTRUN = 0;       // don't call the callback function
    const static int DEFCALL_RUNONLY = 1;       // call the callback function but don't delete the timer
    const static int DEFCALL_RUNANDDEL = 2;     // call the callback function and delete the timer

    // low level function to initialize and enable a new timer
    // returns the timer number (numTimer) on success or
    // -1 on failure (f == NULL) or no free timers
    int setupTimer(unsigned long d, void* f, void* p, bool h, unsigned n);

    // find the first available slot
    int findFirstFreeSlot();

    typedef struct {
      unsigned long prev_millis;        // value returned by the millis() function in the previous run() call
      void* callback;                   // pointer to the callback function
      void* param;                      // function parameter
      bool hasParam;                 // true if callback takes a parameter
      unsigned long delay;              // delay value
      unsigned maxNumRuns;              // number of runs to be executed
      unsigned numRuns;                 // number of executed runs
      bool enabled;                  // true if enabled
      unsigned toBeCalled;              // deferred function call (sort of) - N.B.: only used in run()
    } timer_t;

    timer_t timer[MAX_TIMERS];

    // actual number of timers in use (-1 means uninitialized)
    int numTimers;
};

#endif
