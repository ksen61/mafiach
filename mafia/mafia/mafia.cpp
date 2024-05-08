#include <iostream>
#include <vector>
#include <algorithm> // Для функции shuffle
#include <random>    // Для генерации случайных чисел
#include <chrono>    // Для получения начального значения для генератора случайных чисел
#include <thread>    // Для ожидания

// Роли игроков
enum Role { MAFIA, CIVILIAN };

class Player {
private:
    Role role;
    bool alive;
public:
    Player(Role r) : role(r), alive(true) {}

    Role getRole() const {
        return role;
    }

    bool isAlive() const {
        return alive;
    }

    void kill() {
        alive = false;
    }

    void revive() {
        alive = true;
    }
};

class Game {
private:
    std::vector<Player> players;
    std::vector<int> mafiaKilledPlayers; // Вектор для хранения убитых мафией игроков
    std::vector<int> votedOutPlayers;
    int mafiaCount;
    int civilianCount;
public:
    Game(int playerCount) {
        // Устанавливаем количество мафий равным 1
        int mafiaCount = 1;
        civilianCount = playerCount - mafiaCount;

        // Создаем игроков
        for (int i = 0; i < playerCount; ++i) {
            if (i < mafiaCount)
                players.push_back(Player(MAFIA));
            else
                players.push_back(Player(CIVILIAN));
        }

        // Перемешиваем роли
        std::shuffle(players.begin(), players.end(), std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count()));
    }

    void printRole(int playerIndex) {
        switch (players[playerIndex].getRole()) {
        case MAFIA:
            std::cout << "Вы мафия!\n";
            break;
        case CIVILIAN:
            std::cout << "Вы мирный житель!\n";
            break;
        }
    }

    void mafiaTurn() {
        std::cout << "Город засыпает...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Мафия просыпается...\n";

        // Проверяем, является ли текущий игрок мафией
        if (players[0].getRole() == MAFIA) {
            // Логика выбора игрока, которого мафия убьет
            std::cout << "Выберите номер игрока для убийства: ";
            int target;
            std::cin >> target;
            target--; // Пользователь вводит номер с 1, а индексы начинаются с 0

            if (target < 0 || target >= players.size() || players[target].getRole() == MAFIA || !players[target].isAlive()) {
                std::cout << "Неверный выбор, повторите еще раз.\n";
                mafiaTurn(); // Повторяем ход мафии
                return;
            }

            std::cout << "Мафия выбирает игрока " << target + 1 << " для убийства.\n";
            players[target].kill();

            // Добавляем убитого игрока в вектор мафией убитых игроков
            mafiaKilledPlayers.push_back(target);

            // Удаляем выбранного игрока из вектора
            players.erase(players.begin() + target);
        }
        else {
            // Выбор игрока на рандом для других ролей
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, players.size() - 1);

            int target = dis(gen);
            while (!players[target].isAlive() || players[target].getRole() == MAFIA) {
                target = dis(gen);
            }std::cout << "Мафия выбирает игрока " << target + 1 << " для убийства.\n";

            // Добавляем убитого игрока в вектор мафией убитых игроков
            mafiaKilledPlayers.push_back(target);

            // Удаляем выбранного игрока из вектора
            players.erase(players.begin() + target);
        }

        std::cout << "Мафия засыпает...\n";
    }


    void vote() {
        // Просто заглушка, можно добавить реальную логику голосования
        std::cout << "Голосование...\n";

        // Создаем вектор кандидатов для голосования
        std::vector<int> candidates;
        for (int i = 0; i < players.size(); ++i) {
            if (players[i].isAlive()) {
                bool isMafiaKilled = std::find(mafiaKilledPlayers.begin(), mafiaKilledPlayers.end(), i) != mafiaKilledPlayers.end();
                bool isVotedOut = std::find(votedOutPlayers.begin(), votedOutPlayers.end(), i) != votedOutPlayers.end();
                if (!isMafiaKilled && !isVotedOut) {
                    candidates.push_back(i);
                }
            }
        }

        // Проверка, есть ли кандидаты для голосования
        if (candidates.empty()) {
            std::cout << "Нет кандидатов для голосования. Игра завершается.\n";
            return;
        }

        int targetIndex; // Объявляем переменную targetIndex здесь


        // Если текущий игрок не мафия, то он может проголосовать
        std::cout << "Выберите номер игрока, за которого хотите проголосовать: ";
        int voteIndex;
        std::cin >> voteIndex;
        voteIndex--; // Пользователь вводит номер с 1, а индексы начинаются с 0

        if (voteIndex < 0 || voteIndex >= players.size() || !players[voteIndex].isAlive()) {
            std::cout << "Неверный выбор, повторите еще раз.\n";
            vote(); // Повторяем голосование
            return;
        }

        // Проверяем, не был ли игрок уже убит мафией в текущем или предыдущих голосованиях
        bool isMafiaKilled = std::find(mafiaKilledPlayers.begin(), mafiaKilledPlayers.end(), voteIndex) != mafiaKilledPlayers.end();
        if (isMafiaKilled) {
            std::cout << "Игрок " << voteIndex + 1 << " уже был убит мафией и не может быть выбран для голосования.\n";
            vote(); // Повторяем голосование
            return;
        }

        // Проверяем, не был ли игрок уже выбран на голосовании в текущем раунде
        bool alreadyVoted = std::find(votedOutPlayers.begin(), votedOutPlayers.end(), voteIndex) != votedOutPlayers.end();
        if (alreadyVoted) {
            std::cout << "Вы уже проголосовали за этого игрока. Выберите другого.\n";
            vote(); // Повторяем голосование
            return;
        }std::cout << "Игрок " << voteIndex + 1 << " выбран для голосования.\n";
        targetIndex = voteIndex; // Присваиваем значение voteIndex переменной targetIndex

        // Удаляем выбранного игрока из вектора участников голосования
        candidates.erase(std::remove(candidates.begin(), candidates.end(), targetIndex), candidates.end());
        // Добавляем выбранного игрока в вектор уже проголосовавших
        votedOutPlayers.push_back(targetIndex);
        // Убиваем выбранного игрока
        players[targetIndex].kill();
    }







    void play() {
        bool mafiaWon = false;
        bool civiliansWon = false;

        while (!mafiaWon && !civiliansWon) {
            mafiaTurn();
            vote();

            int mafiaAlive = 0;
            int otherRolesAlive = 0;

            for (int i = 0; i < players.size(); ++i) {
                if (players[i].isAlive()) {
                    switch (players[i].getRole()) {
                    case MAFIA:
                        mafiaAlive++;
                        break;
                    case CIVILIAN:
                        otherRolesAlive++;
                        break;
                    }
                }
            }

            // Проверяем условия победы
            if (mafiaAlive >= otherRolesAlive) {
                mafiaWon = true;
            }
            else if (mafiaAlive == 0) {
                civiliansWon = true;
            }

            // Добавим дополнительные условия победы/поражения
            if (mafiaAlive == 1 && otherRolesAlive == 2) {
                mafiaWon = true;
            }
        }

        if (mafiaWon) {
            std::cout << "Победила мафия!\n";
        }
        else {
            std::cout << "Победили мирные жители!\n";
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    int playerCount;
    std::cout << "Введите количество игроков (от 5 до 7): ";
    std::cin >> playerCount;

    if (playerCount < 5 || playerCount > 7) {
        std::cout << "Неправильное количество игроков!\n";
        return 1;
    }

    Game game(playerCount);
    std::cout << "Ваша роль:\n";
    game.printRole(0); // Предполагаем, что игрок всегда находится под индексом 0
    game.play();

    return 0;
}