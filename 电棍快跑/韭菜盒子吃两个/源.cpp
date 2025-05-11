#include<graphics.h>
#include <windows.h>
#include<string>
#include<vector>

using namespace std;

int idx_current_animation = 0;
const int  PLAYER_ANIM_NUM = 6;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

bool running = true;
bool is_game_started = false;

// 按钮参数
const int CHAR_BTN_WIDTH = 192;      // 按钮宽度
const int CHAR_BTN_HEIGHT = 75;     // 按钮高度

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
IMAGE img_bullet;

#pragma comment(lib, "msimg32.lib")
#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib, "Winmm.lib")

//绘制透明底图片的自定义函数
inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

class Animation
{
public:
	Animation(LPCTSTR path, int num, int interval)
	{
		interval_ms = interval;

		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}

	~Animation()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
			delete frame_list[i];
	}

	void Play(int x, int y, int delta)
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, frame_list[idx_frame]);
	}

private:
	int timer = 0;              //动画计时器
	int idx_frame = 0;          //动画帧索引
	int interval_ms = 0;        //动画帧间隔
	vector<IMAGE*> frame_list;  //图像储存
};

class Player
{
public:
	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(_T("img/player_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/player_right_%d.png"), 6, 45);
		anim_face = new Animation(_T("img/player_%d.png"), 4, 45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)
	{
		if (msg.message == WM_KEYDOWN)
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = 1;
				break;
			case VK_DOWN:
				is_move_down = 1;
				break;
			case VK_LEFT:
				is_move_left = 1;
				break;
			case VK_RIGHT:
				is_move_right = 1;
				break;
			}
		}

		else if (msg.message == WM_KEYUP)
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = 0;
				break;
			case VK_DOWN:
				is_move_down = 0;
				break;
			case VK_LEFT:
				is_move_left = 0;
				break;
			case VK_RIGHT:
				is_move_right = 0;
				break;
			}
		}
		if (is_move_up) player_pos.y -= PlayerSpeed;
		if (is_move_down) player_pos.y += PlayerSpeed;
		if (is_move_left) player_pos.x -= PlayerSpeed;
		if (is_move_right) player_pos.x += PlayerSpeed;
	}

	void Move()
	{
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			player_pos.x += (int)(PlayerSpeed * normalized_x);
			player_pos.y += (int)(PlayerSpeed * normalized_y);
		}

		if (player_pos.x < 0)player_pos.x = 0;
		if (player_pos.y < 0)player_pos.y = 0;
		if (player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH) player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (player_pos.y + PLAYER_HEIGHT > WINDOW_HEIGHT)player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
	}

	void Draw(int delta)
	{
		static bool facing_left = 0;
		static bool facing = 0;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
		{
			facing_left = 1;
			facing = 0;
		}
		else
		{
			facing_left = 0;
			facing = 0;
		}
		if (dir_x == 0)
			facing = 1;
		if (facing_left)
			anim_left->Play(player_pos.x, player_pos.y, delta);
		else if(facing)
			anim_face->Play(player_pos.x, player_pos.y, delta);
		else
			anim_right->Play(player_pos.x, player_pos.y, delta);
	}

	const POINT& GetPosition() const
	{
		return player_pos;
	}

	const int& GetWidth() const
	{
		return PLAYER_WIDTH;
	}

	const int& GetHeight() const
	{
		return PLAYER_HEIGHT;
	}
private:
	const int PLAYER_WIDTH = 160;   //玩家宽度
	const int PLAYER_HEIGHT = 90;  //玩家高度
	const int PlayerSpeed = 3;

private:
	IMAGE img_shadow;
	POINT player_pos = { 500,500 };
	Animation* anim_left;
	Animation* anim_right;
	Animation* anim_face;
	bool is_move_up = 0;
	bool is_move_down = 0;
	bool is_move_left = 0;
	bool is_move_right = 0;
};

class Bullet
{
public:
	POINT position = { 0,0 };
public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		putimage_alpha(position.x, position.y,&img_bullet);
	}
private:
	const int RADIUS = 10;
};

class Enemy
{
public:
	Enemy()
	{
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/enemy_left_%d.png"), 5, 45);
		anim_right = new Animation(_T("img/enemy_right_%d.png"), 5, 45);

		//敌人生成边界
		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		//将敌人放置在地图外边界的随机位置
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		case SpawnEdge::Right:
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		default:
			break;
		}
	}

	bool CheckBulletCollision(const Bullet& bullet)
	{
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	bool CheckPlayerCollision(const Player& player)
	{
		POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
		bool is_overlap_x = check_position.x >= player.GetPosition().x && check_position.x <= player.GetPosition().x + player.GetWidth();
		bool is_overlap_y = check_position.y >= player.GetPosition().y && check_position.y <= player.GetPosition().y + player.GetHeight();
		return is_overlap_x && is_overlap_y;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(Speed * normalized_x);
			position.y += (int)(Speed * normalized_y);
			if (dir_x < 0)
				facing_left = 1;
			else if (dir_x > 0)
				facing_left = 0;
		}
	}

	void Draw(int delta)
	{
		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

	bool ProcessBulletCollision(int bullet_index, bool is_colliding) {
		if (bullet_index < 0 || bullet_index >= 3) return false;

		if (is_colliding) {
			if (!hit_bullets[bullet_index]) {
				hit_bullets[bullet_index] = true;
				return true;
			}
		}
		else {
			hit_bullets[bullet_index] = false;
		}
		return false;
	}

	void Hurt() {
		HealthPoints--;
		if (HealthPoints <= 0) {
			alive = false;
		}
	}

	bool CheckAlive()
	{
		return alive;
	}

	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}

private:
	const int FRAME_WIDTH = 192;   //敌人宽度
	const int FRAME_HEIGHT = 108;  //敌人高度
	const int Speed = 2;

private:
	IMAGE img_shadow;
	POINT position = { 0,0 };
	Animation* anim_left;
	Animation* anim_right;
	bool facing_left = 0;
	bool hit_bullets[3] = { false };
	bool alive = 1;
	int HealthPoints = 3;
};

enum class Scene {
	MENU,
	GAME
};

Scene currentScene = Scene::MENU;

class Button {
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed) {
		region = rect;
		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}

	~Button() = default;

	void ProcessEvent(const ExMessage& msg) {
		switch (msg.message) {
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed && CheckCursorHit(msg.x, msg.y))
				OnClick();
			break;
		default:
			break;
		}
	}

	void Draw() {
		switch (status) {
		case Status::Idle:
			putimage_alpha(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage_alpha(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage_alpha(region.left, region.top, &img_pushed);
			break;
		}
	}

protected:
	virtual void OnClick() = 0;

private:
	enum class Status {
		Idle = 0,
		Hovered,
		Pushed
	};

private:
	//检测鼠标点击
	bool CheckCursorHit(int x, int y) {
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}

private:
	RECT region;//描述位置和大小
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;
};

class StartGameButton :public Button {
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
	}
	~StartGameButton() = default;

protected:
	void OnClick() {
		currentScene = Scene::GAME;
		mciSendString(_T("play bgm from 0"), NULL, 0, NULL);
	}
};

class QuitGameButton :public Button {
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
	}
	~QuitGameButton() = default;

protected:
	void OnClick() {
		running = false;
	}
};

void TryGenerateEnemy(vector<Enemy*>& enemy_list)
{
	int INTERVAL = 80;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
	{ 
		enemy_list.push_back(new Enemy());
		if (INTERVAL > 20) INTERVAL -= 2;
	}
}

void UpDateBullets(vector<Bullet>& bullet_list, const Player& player)
{
	const double RADIAL_SPEED = 0.0045;                           //法向速度
	const double TANGENT_SPEED = 0.0055;                          //切向速度
	double radian_interval = 2 * 3.14159 / bullet_list.size();    //子弹间的弧度间隔
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;//当前子弹所在弧度制
		bullet_list[i].position.x = player_position.x + player.GetWidth() / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.GetHeight() / 2 + (int)(radius * cos(radian));
	}
}

void DrawPlayerScore(int score)
{
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家得分：%d"), score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}

Player player;

RECT btn_rect_start = {
	  (WINDOW_WIDTH - 200) / 2,     
	  (WINDOW_HEIGHT - 80) / 2,     
	  (WINDOW_WIDTH + 200) / 2,     
	  (WINDOW_HEIGHT + 80) / 2      
};
StartGameButton start_btn(btn_rect_start,
	_T("img/btn_start_idle.png"),
	_T("img/btn_start_hovered.png"),
	_T("img/btn_start_pushed.png"));

RECT btn_rect_quit = {
	(WINDOW_WIDTH - 200) / 2,
	(WINDOW_HEIGHT - 80) / 2 + 120,
	(WINDOW_WIDTH + 200) / 2,
	(WINDOW_HEIGHT + 80) / 2 + 120
};
QuitGameButton quit_btn(btn_rect_quit,
	_T("img/btn_quit_idle.png"),
	_T("img/btn_quit_hovered.png"),
	_T("img/btn_quit_pushed.png"));

int main()
{
	initgraph(1280, 720);

	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mus/xuan.wav alias hit"), NULL, 0, NULL);
	mciSendString(_T("open mus/dead.wav alias dead"), NULL, 0, NULL);

	int score = 0;
	ExMessage msg;
	IMAGE img_background;
	IMAGE img_menu_background;
	vector<Enemy*> enemy_list;
	vector<Bullet> bullet_list(3);

	loadimage(&img_background, _T("img/background.png"));
	loadimage(&img_menu_background, _T("img/menu_background.png"));
	loadimage(&img_bullet, _T("img/bullet.png"));

	while (running)
	{
		BeginBatchDraw();

		DWORD start_time = GetTickCount();

		while (peekmessage(&msg, EX_MOUSE | EX_KEY))
		{
			if (currentScene == Scene::MENU)
			{
				start_btn.ProcessEvent(msg);
				quit_btn.ProcessEvent(msg);
			}
			else if (currentScene == Scene::GAME)
			{
				player.ProcessEvent(msg);
			}
		}

		cleardevice();

		switch (currentScene)
		{
		case Scene::MENU:
			putimage(0, 0, &img_menu_background); 
			start_btn.Draw();
			quit_btn.Draw();
			break;
		case Scene::GAME:
			putimage(0, 0, &img_background); 
			player.Move();
			UpDateBullets(bullet_list, player);
			TryGenerateEnemy(enemy_list);
			for (Enemy* enemy : enemy_list)
				enemy->Move(player);
			//敌人与玩家的碰撞
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->CheckPlayerCollision(player))
				{
					mciSendString(_T("play dead from 0"), NULL, 0, NULL);
					TCHAR text[64];
					_stprintf_s(text, _T("欸你怎么死了\n得分：%d"), score);
					MessageBox(GetHWnd(), text, _T("Good game"), MB_OK);
					running = false;
					break;
				}
			}

			//敌人与子弹的碰撞
			for (Enemy* enemy : enemy_list) {
				for (size_t j = 0; j < bullet_list.size(); j++) {
					bool is_colliding = enemy->CheckBulletCollision(bullet_list[j]);
					if (enemy->ProcessBulletCollision(j, is_colliding)) {
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);
						enemy->Hurt();
						
					}
				}
			}
			

			//清除生命值归零的敌人
			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{
					swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					score++;
					delete enemy;
				}
			}

			player.Draw(1000 / 144);
			for (Enemy* enemy : enemy_list)
				enemy->Draw(1000 / 144);
			for (const Bullet& bullet : bullet_list)
				bullet.Draw();
			DrawPlayerScore(score);

			break;
		}

		
		
		FlushBatchDraw();
		EndBatchDraw();

		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;


		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
		
	}

	return 0;
}