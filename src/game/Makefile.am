SUBDIRS = menus

PLAYER_SRC = Player.h Player.cpp Muncher.h Muncher.cpp Troggle.h Troggle.cpp
GAME_SRC = Game.h \
		   Game.cpp \
		   SinglePlayerGame.h \
		   SinglePlayerGame.cpp \
		   MultiPlayerGame.h \
		   MultiPlayerGame.cpp

##MultiPlayerServer.h MultiPlayerServer.cpp

OTHER_SRC = Gnumch.h Gnumch.cpp \
			Board.h Board.cpp \
			Animation.h Animation.cpp \
			FileSys.h FileSys.cpp \
			ConfigFile.h ConfigFile.cpp \
			Event.h Event.cpp

LEVEL_SRC = Level.h Level.cpp \
			PrimeLevel.h PrimeLevel.cpp \
			FactorLevel.h FactorLevel.cpp \
			MultipleLevel.h MultipleLevel.cpp \
			EqualityLevel.h EqualityLevel.cpp

##NET_SRC = NetworkThread.h NetworkThread.cpp ClientNetThread.cpp ServerNetThread.cpp

bin_PROGRAMS = gnumch
gnumch_SOURCES = ${PLAYER_SRC} ${GAME_SRC} ${LEVEL_SRC} ${OTHER_SRC}
gnumch_LDADD = menus/libGnumchMenus.a ../gui/libGui.a
INCLUDES = -I../gui/

localedir=$(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
