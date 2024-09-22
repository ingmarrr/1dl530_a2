L = FLAKY
L0.lock()
...
L0.unlock()

L1.lock()
...
L1.unlock()

-------------------
THREADS     = 1
TURN        = 0
BUSY        = false

L.lock()

ME  = GET_THREAD_ID();      == 0
do
{
    do 
    {
        TURN    = 0                 == TURN = 0
    } while (busy)                  == BUSY = false
    BUSY = true                     == BUSY = true
} while (TURN != ME)                == false

-->

-------------------
THREADS     = 2
TURN        = 0
BUSY        = false

L.lock()

ME_0    = GET_THREAD_ID();      == 0
ME_1    = GET_THREAD_ID();      == 1

do
{
    do 
    {
        TURN_0      = 0             == TURN = 0
        TURN_1      = 1             == TURN = 1
    } while (BUSY)                  == BUSY = false
    BUSY_0 = true                   == BUSY = true
    BUSY_1 = true                   == BUSY = true
} while (TURN != ME)                == TURN == 1

T1 exits and continue in their CS
-->


