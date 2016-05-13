.. default-role:: code

.. contents:: Table of contents:
   :local:
   :depth: 2


.. _Robot Framework User Guide: http://robotframework.org/robotframework/#user-guide


.. code:: robotframework

    *** Test Cases ***

    1. Starting new process
        Execute This    25401
        Status Should Be    0

    2. Reading memory
        Execute This    25402
        Status Should Be    0

    3. Writing to memory
        Execute This    25403
        Status Should Be    0

    4. Starting new process from CMD line
        Execute This    25404
        Status Should Be    0

    5. Terminate process
        Execute This    25405
        Status Should Be    0

    6. Terminate process ASK
        Execute This    25406
        Status Should Be    0

    7. Open running process
        Execute This    25407
        Status Should Be    0

    8. Open running process ASK
        Execute This    25408
        Status Should Be    0

    9. Start new thread
        Execute This    25409
        Status Should Be    0

    10. Start new thread ASK
        Execute This    25410
        Status Should Be    0

    11. Keylogging
        Execute This    25411
        Status Should Be    0

    12. Create File
        Execute This    25412
        Status Should Be    0

    13. Open File
        Execute This    25413
        Status Should Be    0

    14. Read File
        Execute This    25414
        Status Should Be    0

    15. Write File
        Execute This    25415
        Status Should Be    0

    16. Rename File
        Execute This    25416
        Status Should Be    0

    17. Delete File
        Execute This    25417
        Status Should Be    0
		
    18. Create Registry
        Execute This    25418
        Status Should Be    0

    19. Open Registry
        Execute This    25419
        Status Should Be    0

    20. Read Registry
        Execute This    25420
        Status Should Be    0

    21. Write Registry
        Execute This    25421
        Status Should Be    0

    22. Rename Registry
        Execute This    25422
        Status Should Be    0

    23. Delete Registry
        Execute This    25423
        Status Should Be    0

.. code:: robotframework

    *** Settings ***
    Library           OperatingSystem
    Library           lib/LoginLibrary.py





.. code:: robotframework

    *** Settings ***
    Force Tags        quickstart
    Default Tags      example    smoke
