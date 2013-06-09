HEADERS += DiskManager/diskmanager.h \
    DiskManager/disk_drive.h \
    GUI/shredthread.h \
    GUI/shredder.h \
    GUI/pass.h \
    GUI/logger.h \
    GUI/listdialog.h \
    GUI/inputdialog.h \
    GUI/histthread.h \
    GUI/histogram.h \
    GUI/helper.h \
    GUI/aboutdialog.h \
    Scrubber/utility.h \
    Scrubber/scrubpattern.h \
    Scrubber/scrubpass.h \
    Scrubber/scrublogger.h \
    Scrubber/scrubber.h \
    Scrubber/random.h \
    GUI/pattern.h \
    Scrubber/genrand.h \
    Scrubber/aes.h
SOURCES += DiskManager/diskmanager.cpp \
    GUI/shredthread.cpp \
    GUI/shredder.cpp \
    GUI/pass.cpp \
    GUI/main.cpp \
    GUI/logger.cpp \
    GUI/listdialog.cpp \
    GUI/inputdialog.cpp \
    GUI/histthread.cpp \
    GUI/histogram.cpp \
    GUI/helper.cpp \
    GUI/aboutdialog.cpp \
    Scrubber/utility.cpp \
    Scrubber/scrubpattern.cpp \
    Scrubber/scrubpass.cpp \
    Scrubber/scrublogger.cpp \
    Scrubber/scrubber.cpp \
    Scrubber/random.cpp \
    GUI/pattern.cpp \
    Scrubber/genrand.cpp \
    Scrubber/aes.cpp
FORMS += GUI/shredder.ui \
    GUI/listdialog.ui \
    GUI/inputdialog.ui \
    GUI/histogram.ui \
    GUI/aboutdialog.ui
OTHER_FILES += \
    Scripts/get_drives.sh \
    Scripts/get_boot_drive.sh

INCLUDEPATH += /home/erez/Downloads/Shredder/boost_1_53_0
