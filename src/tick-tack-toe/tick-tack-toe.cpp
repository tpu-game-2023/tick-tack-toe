#include <memory>
#include <iostream>
#include <string>

class Mass
{
public:
	enum status
	{
		BLANK,
		PLAYER,
		ENEMY,
	};
private:
	status s_ = BLANK;
public:
	void setStatus(status s) { s_ = s; }
	status getStatus() const { return s_; }

	bool put(status s)
	{
		if (s_ != BLANK) return false;
		s_ = s;
		return true;
	}
};

class Board;

class AI
{
public:
	AI() {}
	virtual ~AI() {}

	virtual bool think(Board& b) = 0;

public:
	enum type
	{
		TYPE_ORDERED = 0,
		TYPE_ALPHABETA = 1,
		TYPE_NEGAMAX = 2,

		INVALID,
	};

	static AI* createAi(type type);
};

// 順番に打ってみる
class AI_ordered : public AI
{
public:
	AI_ordered() {}
	~AI_ordered() {}

	bool think(Board& b);
};

class AI_alpha_beta : public AI
{
private:
	int evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y);

public:
	AI_alpha_beta() {}
	~AI_alpha_beta() {}

	bool think(Board& b);
};

class AI_nega_max : public AI
{
private:
	int evaluate(Board& b, Mass::status current, int& best_x, int& best_y);

public:
	AI_nega_max() {}
	~AI_nega_max() {}

	bool think(Board& b);
};

AI* AI::createAi(type type)
{
	switch (type)
	{
	case TYPE_ORDERED:
		return new AI_ordered();
	case TYPE_ALPHABETA:
		return new AI_alpha_beta();
	case TYPE_NEGAMAX:
		return new AI_nega_max();
	default:
		//その他はエラーを起こしたいためnullptrを返す
		std::cout << "[ERROR]登録されていないTYPEが選択されました。" << std::endl;
		return nullptr;
	}
}

class Board
{
	friend class AI_ordered;
	friend class AI_alpha_beta;
	friend class AI_nega_max;

public:
	enum WINNER
	{
		NOT_FINISHED = 0,
		PLAYER,
		ENEMY,
		DRAW,
	};
private:
	enum
	{
		BOARD_SIZE = 3,
	};
	Mass mass_[BOARD_SIZE][BOARD_SIZE];

public:
	Board()
	{
	}
	Board::WINNER calc_result() const
	{
		// 縦横斜めに同じキャラが入っているか検索
		// 横
		for (int y = 0; y < BOARD_SIZE; y++)
		{
			Mass::status winner = mass_[y][0].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int x = 1;
			for (; x < BOARD_SIZE; x++)
			{
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (x == BOARD_SIZE) { return (Board::WINNER)winner; }
		}
		// 縦
		for (int x = 0; x < BOARD_SIZE; x++)
		{
			Mass::status winner = mass_[0][x].getStatus();
			if (winner != Mass::PLAYER && winner != Mass::ENEMY) continue;
			int y = 1;
			for (; y < BOARD_SIZE; y++)
			{
				if (mass_[y][x].getStatus() != winner) break;
			}
			if (y == BOARD_SIZE) { return(Board::WINNER)winner; }
		}
		// 斜め
		{
			Mass::status winner = mass_[0][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY)
			{
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++)
				{
					if (mass_[idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		{
			Mass::status winner = mass_[BOARD_SIZE - 1][0].getStatus();
			if (winner == Mass::PLAYER || winner == Mass::ENEMY)
			{
				int idx = 1;
				for (; idx < BOARD_SIZE; idx++)
				{
					if (mass_[BOARD_SIZE - 1 - idx][idx].getStatus() != winner) break;
				}
				if (idx == BOARD_SIZE) { return (Board::WINNER)winner; }
			}
		}
		// 上記勝敗がついておらず、空いているマスがなければ引分け
		for (int y = 0; y < BOARD_SIZE; y++)
		{
			for (int x = 0; x < BOARD_SIZE; x++)
			{
				Mass::status fill = mass_[y][x].getStatus();
				if (fill == Mass::BLANK) return NOT_FINISHED;
			}
		}
		return DRAW;
	}

	bool put(int x, int y)
	{
		if (x < 0 || BOARD_SIZE <= x ||
			y < 0 || BOARD_SIZE <= y) return false;// 盤面外
		return mass_[y][x].put(Mass::PLAYER);
	}

	void show() const
	{
		std::cout << "　　";
		for (int x = 0; x < BOARD_SIZE; x++)
		{
			std::cout << " " << x + 1 << "　";
		}
		std::cout << "\n　";
		for (int x = 0; x < BOARD_SIZE; x++)
		{
			std::cout << "＋－";
		}
		std::cout << "＋\n";
		for (int y = 0; y < BOARD_SIZE; y++)
		{
			std::cout << " " << char('a' + y);
			for (int x = 0; x < BOARD_SIZE; x++)
			{
				std::cout << "｜";
				switch (mass_[y][x].getStatus())
				{
				case Mass::PLAYER:
					std::cout << "〇";
					break;
				case Mass::ENEMY:
					std::cout << "×";
					break;
				case Mass::BLANK:
					std::cout << "　";
					break;
				default:
					//					if (mass_[y][x].isListed(Mass::CLOSE)) std::cout << "＋"; else
					//					if (mass_[y][x].isListed(Mass::OPEN) ) std::cout << "＃"; else
					std::cout << "　";
				}
			}
			std::cout << "｜\n";
			std::cout << "　";
			for (int x = 0; x < BOARD_SIZE; x++)
			{
				std::cout << "＋－";
			}
			std::cout << "＋\n";
		}
	}
};

bool AI_ordered::think(Board& b)
{
	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			if (b.mass_[y][x].put(Mass::ENEMY))
			{
				return true;
			}
		}
	}
	return false;
}

int AI_alpha_beta::evaluate(int alpha, int beta, Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	//死活判定
	int r = b.calc_result();
	if (r == current)return +10000;//呼び出し側の勝ち
	if (r == next) return -10000;//呼び出し側の負け
	if (r == Board::DRAW)return 0;//引き分け

	int score_max = -9999;//打たないで投了

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current);//次の手を打つ
			int dummy;
			int score = -evaluate(-beta, -alpha, b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);//手を戻す

			if (beta < score)
			{
				return (score_max < score) ? score : score_max;
			}
			if (score_max < score)
			{
				score_max = score;
				//α値を更新
				alpha = (alpha < score_max) ? score_max : alpha;
				best_x = x;
				best_y = y;
			}
		}
	}
	return score_max;
}

bool AI_alpha_beta::think(Board& b)
{
	int best_x, best_y;

	if (evaluate(-10000, 10000, b, Mass::ENEMY, best_x, best_y) <= -9999)return false;

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

int AI_nega_max::evaluate(Board& b, Mass::status current, int& best_x, int& best_y)
{
	Mass::status next = (current == Mass::ENEMY) ? Mass::PLAYER : Mass::ENEMY;

	//死活判定
	int r = b.calc_result();
	if (r == current)return 10000;//呼び出し側の勝ち
	if (r == next)return -10000;//呼び出し側の負け
	if (r == Board::DRAW)return 0;//引き分け

	int score_max = -9999; //打たないのは最悪

	for (int y = 0; y < Board::BOARD_SIZE; y++)
	{
		for (int x = 0; x < Board::BOARD_SIZE; x++)
		{
			Mass& m = b.mass_[y][x];
			if (m.getStatus() != Mass::BLANK)continue;

			m.setStatus(current);
			int dummy;
			int score = -evaluate(b, next, dummy, dummy);
			m.setStatus(Mass::BLANK);

			if (score_max < score)
			{
				score_max = score;
				best_x = x;
				best_y = y;
			}
		}
	}

	return score_max;
}

bool AI_nega_max::think(Board& b)
{
	int best_x = -1, best_y;

	evaluate(b, Mass::ENEMY, best_x, best_y);

	//打てる手はなかった
	if (best_x < 0)return false;

	return b.mass_[best_y][best_x].put(Mass::ENEMY);
}

class Game
{
private:
	AI::type ai_type;

	Board board_;
	Board::WINNER winner_ = Board::NOT_FINISHED;
	AI* pAI_ = nullptr;

public:
	Game(AI::type type)
	{
		ai_type = type;
		pAI_ = AI::createAi(type);
	}
	~Game()
	{
		delete pAI_;
	}

	bool put(int x, int y)
	{
		bool success = board_.put(x, y);
		if (success) winner_ = board_.calc_result();

		return success;
	}

	bool think()
	{
		bool success = pAI_->think(board_);
		if (success) winner_ = board_.calc_result();
		return success;
	}

	Board::WINNER is_finished()
	{
		return winner_;
	}

	void show()
	{
		board_.show();
	}

	std::string get_ai_type_message()
	{
		switch (ai_type)
		{
		case AI::TYPE_ORDERED:
			return "ORDERED";
		case AI::TYPE_ALPHABETA:
			return "ALPHA_BETA";
		case AI::TYPE_NEGAMAX:
			return "NEGA_MAX";
		default:
			break;
		}

		return "";
	}
};

void show_start_message(std::string ai_type)
{
	std::cout << "========================" << std::endl;
	std::cout << "       GAME START       " << std::endl;
	std::cout << std::endl;
	std::cout << "AI TYPE : " << ai_type << std::endl;
	std::cout << std::endl;
	std::cout << "input position likes 1 a" << std::endl;
	std::cout << "========================" << std::endl;
}

void show_end_message(Board::WINNER winner)
{
	if (winner == Board::PLAYER)
	{
		std::cout << "You win!" << std::endl;
	}
	else if (winner == Board::ENEMY)
	{
		std::cout << "You lose..." << std::endl;
	}
	else
	{
		std::cout << "Draw" << std::endl;
	}
	std::cout << std::endl;
}

void show_type_select_message()
{
	std::cout << "対戦したい <<AI TYPE>> を選択してください。" << std::endl;
	std::cout << "1 → ORDERED" << std::endl;
	std::cout << "2 → ALPHA_BETA" << std::endl;
	std::cout << "3 → NEGA_MAX" << std::endl;

	std::cout << std::endl;
}

AI::type ai_type_setting()
{
	int type;
	AI::type ai_type;
	do
	{
		std::cout << ">";
		std::cin >> type;
		ai_type = static_cast<AI::type>(--type);
	} while (ai_type < AI::TYPE_ORDERED || ai_type >= AI::INVALID);

	return ai_type;
}

int main()
{
	while (true)
	{
		//AITYPEの設定
		show_type_select_message();

		AI::type ai_type = ai_type_setting();

		//ゲームインスタンスを生成
		std::shared_ptr<Game> game(new Game(ai_type));

		show_start_message(game->get_ai_type_message());

		unsigned int turn = 0;

		while (true)
		{
			game->show();// 盤面表示

			// 勝利判定
			Board::WINNER winner = game->is_finished();
			if (winner)
			{
				show_end_message(winner);
				break;
			}

			if (turn == 0)
			{
				// user input
				char col[1], row[1];
				do
				{
					std::cout << "? ";
					std::cin >> row >> col;
				} while (!game->put(row[0] - '1', col[0] - 'a'));
			}
			else
			{
				// AI
				if (!game->think())
				{
					show_end_message(Board::WINNER::PLAYER);// 投了
				}
				std::cout << std::endl;
			}
			// プレイヤーとAIの切り替え
			turn = 1 - turn;
		}
	}

	return 0;
}