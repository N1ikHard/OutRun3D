#include<SFML/Graphics.hpp>
#include<time.h>
using namespace sf;

int width = 1024;
int height = 768;
int roadW = 2000;
int segL = 200; //Размер сегмента
float camD = 0.84;//


struct Line {

	float x, y, z; //3d центр линии
	float X, Y, W; 
	float scale , curve , clip , spriteX;

	Sprite sprite;
	
	Line() 
	{
		spriteX=curve=x = y = z = 0;
	}

	void project(int camX, int  camY, int camZ) {

		scale = camD / (z - camZ);
		X = (1 + scale * (x - camX)) * width / 2;
		Y = (1 - scale * (y - camY)) * height / 2;
		W = scale * roadW * width / 2;
	}

	void drawSprite(RenderWindow& app) 
	{
	
		Sprite s = sprite;
		int w = s.getTextureRect().width;
		int	 h = s.getTextureRect().height;

		float destX = X + scale * spriteX * width / 2;
		float destY = Y + 4;
		float destW = w * W / 266;
		float destH = h * W / 266;
		destX += destW * spriteX; //offsetX
		destY += destH * (-1);    //offsetY

		float clipH = destY + destH - clip;
		if (clipH < 0) clipH = 0;

		if (clipH >= destH) return;
		s.setTextureRect(IntRect(0, 0, w, h - h * clipH / destH));
		s.setScale(destW / w, destH / h);
		s.setPosition(destX, destY);
		app.draw(s);
	}

};

//Метод , для рисования трапеции
void drawQuad(RenderWindow& w, Color c, int x1, int y1, int w1, int x2, int y2, int w2) {
	
	ConvexShape shape(4);	//Четырехугольник

	shape.setFillColor(c); //Выбираем цвет

	//Устанавливаем координаты сторон
	shape.setPoint(0, Vector2f(x1 - w1, y1));
	shape.setPoint(1, Vector2f(x2 - w2, y2));
	shape.setPoint(2, Vector2f(x2 + w2, y2));	  
	shape.setPoint(3, Vector2f(x1 + w1, y1));	  

	//Отображаем на экране
	w.draw(shape);
}




 int main() {

	 RenderWindow app(VideoMode(width ,height), "OutRun");
	 app.setFramerateLimit(60);

	 Texture t[50];
	 Sprite object[50];

	 //Установка разных текстур
	 for (int i = 1; i <= 7; i++) 
	 {
		 t[i].loadFromFile("images/" + std::to_string(i) + ".png");
		 t[i].setSmooth(true);											
		 object[i].setTexture(t[i]);
	 }


	 //Установка текстур и спрайта для фона
	 Texture bg;
	 bg.loadFromFile("images/bg.png");
	 bg.setRepeated(true);
	 Sprite sBackground(bg);  
	 sBackground.setTextureRect(IntRect(0, 0, 5000, 411));
	 sBackground.setPosition(-2000, 0);


	 //Список из линий
	 std::vector<Line>lines;
	 for (int i = 0; i < 1600; i++)
	 {
		 Line line;
		 line.z = i * segL;

		 //Изгиб вправо
		 if (i > 300 && i < 700)
			 line.curve = 0.5;

		 //Изгиб влево
		 if (i > 1100) line.curve = -0.7;

		 if (i < 300 && i % 20 == 0) { line.spriteX = -2.5; line.sprite = object[5]; }	    //Спрайт Дерево
		 if (i % 17 == 0) { line.spriteX = 2.0; line.sprite = object[6]; }					//Спрайт Кусты
		 if (i > 300 && i % 20 == 0) { line.spriteX = -0.7; line.sprite = object[4]; }	    //Еще пальма
		 if (i > 800 && i % 20 == 0) { line.spriteX = -1.2; line.sprite = object[1]; }		//Спрайт пальма под наклоном
		 if (i == 400) { line.spriteX = -1.2; line.sprite = object[7]; }					//Спрайт мастерская


		 if (i > 750) line.y = sin(i / 30.0) * 1500;

   		 lines.push_back(line);
	 }

	 int N = lines.size();
	 int pos = 0;
	 int playerX = 0;
	 int H = 1500;

	 while (app.isOpen()) {

		 Event e;
		 while (app.pollEvent(e))
			 if (e.type == Event::Closed)
				 app.close();

		 
		 int speed=0;

		 //Управление
		 if (Keyboard::isKeyPressed(Keyboard::Right)) playerX += 0.1;
		 if (Keyboard::isKeyPressed(Keyboard::Left)) playerX -= 0.1;
		 if (Keyboard::isKeyPressed(Keyboard::Up)) speed = 200;
		 if (Keyboard::isKeyPressed(Keyboard::Down)) speed = -200;
		 if (Keyboard::isKeyPressed(Keyboard::Tab)) speed *= 3;
		 if (Keyboard::isKeyPressed(Keyboard::W)) H += 100;
		 if (Keyboard::isKeyPressed(Keyboard::S)) H -= 100;

		 pos += speed;
		 while (pos >= N * segL) pos -= N * segL;
		 while (pos < 0) pos += N * segL;

		 app.clear(Color(105, 205, 4));
		 app.draw(sBackground);
		 int startPos = pos / segL;
		 int camH = lines[startPos].y + H;
		 if (speed > 0) sBackground.move(-lines[startPos].curve * 2, 0);
		 if (speed < 0) sBackground.move(lines[startPos].curve * 2, 0);

		 int maxy = height;
		 float x = 0, dx = 0;

		 ///////draw road////////
		 for (int n = startPos; n < startPos + 300; n++)
		 {
			 Line& l = lines[n % N];
			 l.project(playerX * roadW - x, camH, startPos * segL - (n >= N ? N * segL : 0));
			 x += dx;
			 dx += l.curve;

			 l.clip = maxy;
			 if (l.Y >= maxy) continue;
			 maxy = l.Y;

			 Color grass = (n / 3) % 2 ? Color(16, 200, 16) : Color(0, 154, 0);				//трава
			 Color rumble = (n / 3) % 2 ? Color(255, 255, 255) : Color(0, 0, 0);			//Белый-черный
			 Color road = (n / 3) % 2 ? Color(107, 107, 107) : Color(105, 105, 105);		//Серый и чуть светлее

			 Line p = lines[(n - 1) % N]; //previous line

			 drawQuad(app, grass, 0, p.Y, width, 0, l.Y, width);							//Рисовка травы
			 drawQuad(app, rumble, p.X, p.Y, p.W * 1.2, l.X, l.Y, l.W * 1.2);				//Рисовка границы
			 drawQuad(app, road, p.X, p.Y, p.W, l.X, l.Y, l.W);								//Рисовка дороги
		 }

		 ////////draw objects////////
		 for (int n = startPos + 300; n > startPos; n--)								    
			 lines[n % N].drawSprite(app);

		 app.display();
	 }

	 return 0;
}