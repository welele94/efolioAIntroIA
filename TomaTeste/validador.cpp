#include <bits/stdc++.h>
using namespace std;

static string trim_crlf(string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    return s;
}

static string read_instance(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) throw runtime_error("nao foi possivel abrir instancia: " + path);
    string s((istreambuf_iterator<char>(f)), istreambuf_iterator<char>());
    string out;
    for (char ch : s) {
        if (ch != '\n' && ch != '\r') out.push_back(ch);
    }
    if (out.size() != 64) {
        throw runtime_error("instancia com tamanho invalido (" + to_string(out.size()) + "): " + path);
    }
    return out;
}

static bool is_piece(char c) {
    return c == 'C' || c == 'T' || c == 'B' || c == 'D';
}

static bool is_white(char c) {
    return c == 'P' || c == 'T' || c == 'B' || c == 'C' || c == 'D' || c == 'R';
}

static int pos_from_square(const string& sq) {
    if (sq.size() < 2) return -1;
    char col = (char)tolower(sq[0]);
    char row = sq[1];
    if (col < 'a' || col > 'h' || row < '1' || row > '8') return -1;
    return (row - '1') * 8 + (col - 'a');
}

static bool adjacent(int a, int b) {
    int ar = a / 8, ac = a % 8;
    int br = b / 8, bc = b % 8;
    return max(abs(ar - br), abs(ac - bc)) == 1;
}

static bool knight_capture(int from, int to) {
    int dr = abs(from / 8 - to / 8);
    int dc = abs(from % 8 - to % 8);
    return (dr == 2 && dc == 1) || (dr == 1 && dc == 2);
}

static bool sliding_capture(const string& board, int from, int to, const vector<pair<int,int>>& dirs) {
    int fr = from / 8, fc = from % 8;
    int tr = to / 8, tc = to % 8;
    for (auto [dr, dc] : dirs) {
        int r = fr + dr, c = fc + dc;
        while (r >= 0 && r < 8 && c >= 0 && c < 8) {
            int p = r * 8 + c;
            if (p == to) return board[p] == 'p';
            if (board[p] != ' ') return false;
            r += dr;
            c += dc;
        }
    }
    return false;
}

static bool can_capture(const string& board, int from, int to) {
    char p = board[from];
    if (to < 0 || to >= 64 || board[to] != 'p') return false;
    if (p == 'C') return knight_capture(from, to);

    static const vector<pair<int,int>> rook = {{1,0},{-1,0},{0,1},{0,-1}};
    static const vector<pair<int,int>> bishop = {{1,1},{1,-1},{-1,1},{-1,-1}};
    static const vector<pair<int,int>> queen = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};

    if (p == 'T') return sliding_capture(board, from, to, rook);
    if (p == 'B') return sliding_capture(board, from, to, bishop);
    if (p == 'D') return sliding_capture(board, from, to, queen);
    return false;
}

static vector<string> split_actions(const string& s) {
    vector<string> out;
    stringstream ss(s);
    string x;
    while (ss >> x) out.push_back(x);
    return out;
}

struct CheckResult {
    bool ok = false;
    int valid_actions = 0;
    string msg;
};

static CheckResult validate_solution(string board, const vector<string>& actions) {
    CheckResult res;
    if (actions.empty()) {
        res.msg = "solucao vazia";
        return res;
    }
    if (actions.size() > 100) {
        res.msg = "mais de 100 acoes";
        return res;
    }

    int active = -1;
    int king = -1;

    for (int i = 0; i < (int)actions.size(); ++i) {
        int dest = pos_from_square(actions[i]);
        if (dest < 0) {
            res.msg = "casa invalida: " + actions[i];
            return res;
        }

        if (i == 0) {
            if (!is_piece(board[dest])) {
                res.msg = "primeira acao nao ativa uma peca: " + actions[i];
                return res;
            }
            active = dest;
            res.valid_actions++;
            continue;
        }

        if (active >= 0) {
            if (can_capture(board, active, dest)) {
                char piece = board[active];
                board[active] = ' ';
                board[dest] = piece;
                active = dest;
                res.valid_actions++;
                continue;
            }

            // sair da peca ativa e mover como rei uma casa
            if (!adjacent(active, dest)) {
                res.msg = "movimento invalido a partir da peca ativa: " + actions[i];
                return res;
            }
            if (board[dest] == ' ') {
                board[dest] = 'R';
                king = dest;
                active = -1;
                res.valid_actions++;
                continue;
            }
            if (is_piece(board[dest])) {
                active = dest;
                king = -1;
                res.valid_actions++;
                continue;
            }
            res.msg = "rei tentou mover para casa ocupada invalida: " + actions[i];
            return res;
        }

        if (king >= 0) {
            if (!adjacent(king, dest)) {
                res.msg = "movimento invalido do rei: " + actions[i];
                return res;
            }
            if (board[dest] == ' ') {
                board[king] = ' ';
                board[dest] = 'R';
                king = dest;
                res.valid_actions++;
                continue;
            }
            if (is_piece(board[dest])) {
                board[king] = ' ';
                king = -1;
                active = dest;
                res.valid_actions++;
                continue;
            }
            res.msg = "rei tentou entrar em casa invalida: " + actions[i];
            return res;
        }

        res.msg = "estado interno invalido";
        return res;
    }

    if (board.find('p') != string::npos) {
        res.msg = "solucao incompleta";
        return res;
    }

    res.ok = true;
    res.msg = "OK";
    return res;
}

static vector<string> split_csv_line(const string& line) {
    vector<string> cols;
    string cur;
    for (char ch : line) {
        if (ch == ';') {
            cols.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(ch);
        }
    }
    cols.push_back(cur);
    return cols;
}

int main(int argc, char** argv) {
    string prefix = "instancias/instancia_";
    string solbase = "resultados";
    string outbase = "validacao";
    int first = 1, last = 10;

    if (argc >= 2) {
        string range = argv[1];
        size_t p = range.find(':');
        if (p != string::npos) {
            first = stoi(range.substr(0, p));
            last = stoi(range.substr(p + 1));
        }
    }

    for (int i = 2; i < argc; ++i) {
        string a = argv[i];
        if (a == "-F" && i + 1 < argc) prefix = argv[++i];
        else if (a == "-S" && i + 1 < argc) solbase = argv[++i];
        else if (a == "-R" && i + 1 < argc) outbase = argv[++i];
    }

    string solfile = solbase;
    if (solfile.size() < 4 || solfile.substr(solfile.size() - 4) != ".csv") solfile += ".csv";

    ifstream fin(solfile);
    if (!fin) {
        cerr << "Nao foi possivel abrir " << solfile << "\n";
        return 1;
    }

    map<int,string> solucoes;
    string line;
    bool firstLine = true;
    while (getline(fin, line)) {
        line = trim_crlf(line);
        if (line.empty()) continue;
        vector<string> cols = split_csv_line(line);
        if (firstLine && !cols.empty() && !cols[0].empty() && !isdigit((unsigned char)cols[0][0])) {
            firstLine = false;
            continue;
        }
        firstLine = false;
        if (cols.size() < 3) continue;
        int id = stoi(cols[0]);
        solucoes[id] = cols[2];
    }

    string outfile = outbase;
    if (outfile.size() < 4 || outfile.substr(outfile.size() - 4) != ".csv") outfile += ".csv";
    ofstream fout(outfile);
    fout << "Instancia;Valida;AcoesValidas;Mensagem\n";

    int ok = 0, total = 0;
    for (int id = first; id <= last; ++id) {
        total++;
        try {
            string inst = read_instance(prefix + to_string(id) + ".txt");
            vector<string> actions = split_actions(solucoes[id]);
            CheckResult r = validate_solution(inst, actions);
            if (r.ok) ok++;
            fout << id << ';' << (r.ok ? 1 : 0) << ';' << r.valid_actions << ';' << r.msg << "\n";
            cout << "Instancia " << id << ": " << (r.ok ? "OK" : "INVALIDA") << " - " << r.msg << "\n";
        } catch (const exception& e) {
            fout << id << ";0;0;" << e.what() << "\n";
            cout << "Instancia " << id << ": ERRO - " << e.what() << "\n";
        }
    }

    cout << "\nValidadas: " << ok << "/" << total << "\n";
    cout << "Ficheiro gerado: " << outfile << "\n";
    return ok == total ? 0 : 2;
}
