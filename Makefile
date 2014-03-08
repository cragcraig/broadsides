TARGET=broadsides
CXXFLAGS=-g
OBJS=main.o agent.o reef.o ship.o playership.o world.o screen.o img.o statetable.o

$(TARGET): $(OBJS)
	g++ $(OBJS) $(CXXFLAGS) -o $(TARGET) `allegro-config --libs`

clean:
	rm -f *.o $(TARGET)
