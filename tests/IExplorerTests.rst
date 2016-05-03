.. default-role:: code

.. contents:: Table of contents:
   :local:
   :depth: 2


.. _Robot Framework User Guide: http://robotframework.org/robotframework/#user-guide


.. code:: robotframework

    *** Test Cases ***

    1. Starting new process
        Execute This    35401
        Status Should Be    0

    2. Reading memory
        Execute This    35402
        Status Should Be    0

    3. Writing to memory
        Execute This    35403
        Status Should Be    0

    4. Starting new process from CMD line
        Execute This    35404
        Status Should Be    0

    5. Terminate process
        Execute This    35405
        Status Should Be    0

    6. Terminate process ASK
        Execute This    35406
        Status Should Be    0

    7. Open running process
        Execute This    35407
        Status Should Be    0

    8. Open running process ASK
        Execute This    35408
        Status Should Be    0

    9. Start new thread
        Execute This    35409
        Status Should Be    0

    10. Start new thread ASK
        Execute This    35410
        Status Should Be    0

    11. Keylogging
        Execute This    35411
        Status Should Be    0

    12. Create File
        Execute This    35412
        Status Should Be    0

    13. Open File
        Execute This    35413
        Status Should Be    0

    14. Read File
        Execute This    35414
        Status Should Be    0

    15. Write File
        Execute This    35415
        Status Should Be    0

    16. Rename File
        Execute This    35416
        Status Should Be    0

    17. Delete File
        Execute This    35417
        Status Should Be    0
		
    18. Create Registry
        Execute This    35418
        Status Should Be    0

    19. Open Registry
        Execute This    35419
        Status Should Be    0

    20. Read Registry
        Execute This    35420
        Status Should Be    0

    21. Write Registry
        Execute This    35421
        Status Should Be    0

    22. Rename Registry
        Execute This    35422
        Status Should Be    0

    23. Delete Registry
        Execute This    35423
        Status Should Be    0

.. code:: robotframework

    *** Settings ***
    Library           OperatingSystem
    Library           lib/LoginLibrary.py





.. code:: robotframework

    *** Settings ***
    Force Tags        quickstart
    Default Tags      example    smoke
