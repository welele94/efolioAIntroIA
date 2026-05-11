#include <stdio.h>


CToma::CToma(void)
{
	tabuleiro.Count(65);
	tabuleiro.Reset(' ');
	tabuleiro.Last() = 0;
}

CToma::~CToma(void)
{}

TProcuraConstrutiva* CToma::Duplicar(void)
{
	CToma* clone = new CToma;
	if (clone != NULL) {
		clone->Copiar(this);
	}
	else
		memoriaEsgotada = true;
	return clone;
}

void CToma::Inicializar(void)
{
	TVector<TString> exemplos = { // https://ekoshapu.in/2022/01/09/puzzles-pawn-mower/
		"                          p         p    ppC              p     ",
		"      p             p      p          p        B      p         ",
		"     p  p    T                                          p    p p",
		"                   p    p  D p                p    p            ",
		"   p         p      pp p     pp    C  p     pp                  ",
		"            p        p p  p p      B p    p p               p p ",
		"         p  pp p                             p   p  pTp  p p    ",
		"p        D  p p                   p    p     p  pp     p   p    "
	};

	TProcuraConstrutiva::Inicializar();
	tabuleiro.Reset(' ');
	tabuleiro.Last() = 0;
	ultimaPeca = atual = -1;
	if (ficheiroInstancia.Empty()) {
		if (instancia.valor <= 8)
			tabuleiro = exemplos[instancia.valor - 1];
		else
			Gerador(instancia.valor);
	}
	else {
		TVector<TString> linhas =
			TString().printf("%s%d.txt", *ficheiroInstancia, instancia.valor)
				.readLines();
		if (linhas.Count() > 0) {
			tabuleiro = *linhas.First();
			if (tabuleiro.Count() != 65) {
				tabuleiro.Count(65);
				tabuleiro.Reset(' ');
				tabuleiro.Last() = 0;
			}
		}
	}
	// atualizar a posição das peças
	pecas = {};
	for (int i = 0; i < 64; i++)
		if (strchr("CBTD", tabuleiro[i]) != NULL)
			pecas += i;
	if (Parametro(TIPO_HEUR) > 3)
		Possivel(); // calculo inicial
}

// gerador de instâncias (mesmo ID e P3 define a mesma instância)
void CToma::Gerador(int id) {
	// usar id para definir a configuração inicial do tabuleiro, garantindo que há solução: 1 a 112 (primeiras 8 são os exemplos, depois 104 gerados)
	// definir número de cada peça: C, B, T, D.
	// definir número de peões pretos, colocar com base em movimentos de peças, para garantir que há solução

	// desperdiçar id valores aleatórios, para que instâncias com a mesma semente mas IDs disintos, não estejam relacionadas
	for (int i = 0; i < id; i++)
		TRand::rand();

	// TODO:
	// - definir peões brancos <<<
	// - limitar movimentos numa só peça? Talvez não, se os peões brancos tratarem de impedir isso
	// - mover com o rei entre peças no gerador? Talvez não, deve conseguir ir entrar numa peça excepto se tiver cercada por peças que deva comer

	// número total de peças = id % 8 + 1 (até 8 peças, distribuindo-se por C,B,T,D)
	int numPecas = ((id - 1) % 8) + 1;
	// número de peões pretos = (id / 8) * 4 (até 56 peões, deixando espaço para até 8 peças)
	int numPeoes = ((id - 1) / 8) * 4;
	TVector<char> posPecas;

	// distribuir numPeoes/2 como peões brancos (podem-se sobrepor)
	for (int i = 0; i < numPeoes / 2; i++)
		tabuleiro[posPecas.Add(TRand::rand() % 64).Last()] = 'P';

	// distribuir peças brancas aleatoriamente (podem-se sobrepor)
	for (int i = 0; i < numPecas; i++) {
		int c = "CBTD"[TRand::rand() % 4];
		int indice = TRand::rand() % 64;
		// não juntar em duas instruções para garantir a ordem da sequência aleatória 
		tabuleiro[posPecas.Add(indice).Last()] = c;
	}

	// remover sobreposições
	posPecas.BeASet();

	// colocar peões pretos gerando movimentos inversos das peças brancas, para garantir que há solução
	for (int i = 0; i < numPeoes; i++) {
		bool pecaColocada = false;
		// escolher peça branca aleatória
		for (auto& posPeca : posPecas.RandomOrder()) {
			// escolher movimento aleatório da peça
			TVector<int> movs = Movimentos(posPeca, tabuleiro[posPeca]).RandomOrder();
			while (movs.Count() > 0 && tabuleiro[movs.Last()] != ' ')
				movs.Pop(); // remover movimentos que não vão para casas vazias
			if (!movs.Empty()) {
				tabuleiro[movs.Last()] = tabuleiro[posPeca];
				tabuleiro[posPeca] = 'p'; // colocar peão na posição da peça, e a peça na posição do movimento
				posPeca = movs.Last(); // atualizar posição da peça para a nova posição
				pecaColocada = true;
				break;
			}
		}
		// peças sem movimentos válidos, não é possível colocar mais peões, parar o processo
		if (!pecaColocada)
			break;
	}
}


void CToma::Gravar(void)
{
	TString().printf("%s%d.txt", *ficheiroGravar, instancia.valor)
		.writeLines({ tabuleiro });
}

void CToma::ResetParametros()
{
	TProcuraConstrutiva::ResetParametros();

	// parametros críticos:
	Parametro(ESTADOS_REPETIDOS) = 3; // usar hashtable, ou então ascendentes, já que há movimentos inversos do rei
	Parametro(BARALHAR_SUCESSORES) = 0; // pelas análises não ajuda baralhar sucessores
	Parametro(ALGORITMO) = BRANCH_AND_BOUND; // pelas análises, o algoritmo Branch-and-Bound é o mais eficiente
	Parametro(LIMITE) = 100; // para o Branch-and-Bound, um limite baixo é melhor, para forçar poda agressiva

	parametro += {
		{"TIPO_HEUR", 4, 0, 5, "tipo de heurística em utilização",
			{ "sem heurística", "# peões", "# peões + distReiA","# peões + distReiB",
			"# peões + distReiB + imp","# peões + distReiB + imp + *(total tomas)" }}, // valor 4 é o mais equilibrado
		{ "PRECALCULAR", 0, 0, 1, "precalcular o mais possível" } // não ajuda
	};

	instancia = { "", 1,1,112 };
}

void CToma::Sucessores(TVector<TNo>& sucessores)
{
	if (atual < 0) {
		// início, ativar uma das peças possíveis
		for (auto peca : pecas) {
			CToma* sucessor = (CToma*)Duplicar();
			if (memoriaEsgotada)
				return;
			sucessor->ultimaPeca = sucessor->atual = peca;
			sucessores += sucessor;
		}
	}
	else if (!SolucaoCompleta()) {
		int pecaID = pecas.Find(atual);
		// ver se há ações de movimentos da peça atual
		if (pecaID >= 0) {
			bool podeTomar = false;
			for (auto mov : Movimentos(atual, tabuleiro[atual])) {
				// fazer movimento se for tomar um peão preto
				if (tabuleiro[mov] == 'p') {
					CToma* sucessor = (CToma*)Duplicar();
					if (memoriaEsgotada)
						return;
					sucessor->tabuleiro[mov] = sucessor->tabuleiro[atual];
					sucessor->tabuleiro[atual] = ' '; // a atual fica vazia
					sucessor->ultimaPeca = sucessor->atual = mov;
					sucessor->pecas[pecaID] = mov;
					if (Parametro(TIPO_HEUR) > 3)
						sucessor->peoesPretos.SetBit(mov, false);
					sucessores += sucessor;
					podeTomar = true;
				}
			}
		}

		// gerar movimentos do Rei (na peça atual ou fora dela), para permitir troca de peça
		if (pecas.Count() > 1 || tabuleiro[atual] == 'R')
			for (auto mov : Movimentos(atual, 'R')) {
				// fazer movimento se for para um espaço branco (mover-se de uma peça para outra), ou para uma peça (ativar a peça)
				if (tabuleiro[mov] == ' ' || strchr("CBTD", tabuleiro[mov]) != NULL) {
					CToma* sucessor = (CToma*)Duplicar();
					if (memoriaEsgotada)
						return;
					if (sucessor->tabuleiro[mov] == ' ') // a nova casa fica com o rei, se estava vazia, c.c. fica com a peça que lá estava
						sucessor->tabuleiro[mov] = 'R';
					if (sucessor->tabuleiro[atual] == 'R')
						sucessor->tabuleiro[atual] = ' '; // a atual fica vazia se estava o rei, mas se for uma peça, fica a pena no mesmo sítio
					sucessor->atual = mov;
					if (sucessor->tabuleiro[mov] != 'R') // entrou numa peça
						sucessor->ultimaPeca = sucessor->atual;
					sucessores += sucessor;
				}
			}
	}

	if (Parametro(TIPO_HEUR) > 3) {
		static TVector<TString> tabuleirosImp;
		// validar todos os sucessores
		for (auto& suc : sucessores)
			if (!((CToma*)suc)->Possivel()) {
				delete suc;
				suc = NULL;
			}
		sucessores -= NULL;
	}

	TProcuraConstrutiva::Sucessores(sucessores);
}

// não há peões pretos, solução completa
bool CToma::SolucaoCompleta(void) {
	if (Parametro(TIPO_HEUR) > 3)
		return !peoesPretos;
	return strchr(tabuleiro.Data(), 'p') == NULL;
}


bool CToma::Distinto(TNo estado) {
	CToma* outro = (CToma*)estado;
	return tabuleiro != outro->tabuleiro || atual != outro->atual;
}


// estimativa até limpar todos os peões:
// 0 sem heurística
// 1 # peões
// 2 # peões + distReiA
// 3 # peões + distReiB
// 4 # peões + distReiB + trocas
int CToma::Heuristica(void)
{
	heuristica = 0;
	if (Parametro(TIPO_HEUR) > 0) { // cc: sem heurística
		// heurística base: número de peões pretos
		int peoesPretos = 0;
		for (auto& c : tabuleiro)
			if (c == 'p')
				peoesPretos++;
		heuristica += peoesPretos;

		if (Parametro(TIPO_HEUR) > 1) { // cc: # peões
			int distanciaRei = 0;

			if (Parametro(TIPO_HEUR) == 2) {
				// distReiA: caso o rei esteja fora, adicionar a distância do rei à peça mais próxima
				if (tabuleiro[atual] == 'R') {
					distanciaRei = 7;
					for (int i = 0; i < 64; i++)
						if (i != atual && strchr("CBTD", tabuleiro[i]) != NULL) {
							int distanciaH = abs(atual % 8 - i % 8);
							int distanciaV = abs(atual / 8 - i / 8);
							int maiorDist = (distanciaH > distanciaV ? distanciaH : distanciaV);
							if (distanciaRei > maiorDist)
								distanciaRei = maiorDist;
						}
				}
			}
			else {
				// distReiB: igual a distReiA mas não conta com a peça de onde vem o rei,
				//           nem peças que não possam tomar peões
				if (!pecas.Empty() && (tabuleiro[atual] == 'R' || pecas.Find(atual) < 0)) {
					distanciaRei = 7;
					for (auto peca : pecas) {
						if (peca != ultimaPeca) {
							int distanciaH = abs(atual % 8 - peca % 8);
							int distanciaV = abs(atual / 8 - peca / 8);
							int maiorDist = (distanciaH > distanciaV ? distanciaH : distanciaV);
							if (distanciaRei > maiorDist)
								distanciaRei = maiorDist;
						}
					}
				}
				if (Parametro(TIPO_HEUR) > 4 && tomaPecas.Count() > 1) {
					// heurística não admissível com base em tomaPecas[]
					// quanto mais peões possam ser tomados por peças distintas melhor
					// somar simplesmente o número de bits (peões) que podem ser tomados por peças
					int totalTomas = 0, totalPeoes = 0;
					for (int i = 0; i < 64; i++)
						if (CToma::peoesPretos.GetBit(i))
							totalPeoes++;
					if (totalPeoes > 1) {
						for (auto& tomas : tomaPecas)
							for (int i = 0; i < 64; i++)
								if (tomas.GetBit(i))
									totalTomas++;
						// situação ideal: totalPeoes*tomaPecas.Count()==totalTomas
						// situação mais desfavorável: totalPeoes==totalTomas
						heuristica += 8 - 8 * (totalTomas - totalPeoes) / (totalPeoes * tomaPecas.Count() - totalPeoes);
					}
				}
			}

			heuristica += distanciaRei;
		}
	}
	return TProcuraConstrutiva::Heuristica();
}

TVector<int> CToma::Movimentos(int pos, char peca, bool saltaPecas) {
	const int direcoesC[8][2] = { {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1} };
	const int direcoesB[4][2] = { {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
	const int direcoesT[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
	static TVector<TVector<int>> movC, movR;
	TVector<int> movs;
	if (pos < 0 || pos >= 64)
		return movs;
	int x = pos % 8;
	int y = pos / 8;
	// cavalo e rei podem estar pre-calculados (restantes não devido a parte dos movimentos poderem estar bloqueados)
	if (Parametro(PRECALCULAR) && (movC.Empty() || movR.Empty())) {
		movC.Count(64);
		movR.Count(64);
		Parametro(PRECALCULAR) = 0;
		for (int i = 0; i < 64; i++) {
			movC[i] = Movimentos(i, 'C');
			movR[i] = Movimentos(i, 'R');
		}
		Parametro(PRECALCULAR) = 1;
	}

	switch (peca) {
	case 'C': // Cavalo
		if (Parametro(PRECALCULAR))
			return movC[pos];
		for (int i = 0; i < 8; i++) {
			int nx = x + direcoesC[i][1];
			int ny = y + direcoesC[i][0];
			if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8)
				movs += ny * 8 + nx;
		}
		break;
	case 'B': // Bispo (não salta peças, parar assim que existir uma peça no caminho)
	case 'D': // Dama (mesmo movimento do bispo, mas também pode se mover como torre)
		for (int i = 0; i < 4; i++) {
			for (int d = 1; d < 8; d++) {
				int nx = x + direcoesB[i][1] * d;
				int ny = y + direcoesB[i][0] * d;
				if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8)
					break; // fora do tabuleiro
				movs += ny * 8 + nx;
				if (saltaPecas ? strchr(" CBTDR", tabuleiro[movs.Last()]) == NULL : tabuleiro[movs.Last()] != ' ')
					break; // peça no caminho, não pode continuar
			}
		}
		if (peca == 'B')
			break;
	case 'T': // Torre
		for (int i = 0; i < 4; i++) {
			for (int d = 1; d < 8; d++) {
				int nx = x + direcoesT[i][1] * d;
				int ny = y + direcoesT[i][0] * d;
				if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8)
					break; // fora do tabuleiro
				movs += ny * 8 + nx;
				if (saltaPecas ? strchr(" CBTDR", tabuleiro[movs.Last()]) == NULL : tabuleiro[movs.Last()] != ' ')
					break; // peça no caminho, não pode continuar
			}
		}
		break;
	case 'R': // Rei (pode se mover para qualquer casa adjacente)
		if (Parametro(PRECALCULAR))
			return movR[pos];
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx == 0 && dy == 0)
					continue; // mesma posição
				int nx = x + dx;
				int ny = y + dy;
				if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8)
					movs += ny * 8 + nx;
			}
		}
		break;
	}
	return movs;
}

bool CToma::Possivel() {
	TBits tomaveis;
	tomaveis.Count(1).Reset(0);
	// atualiza os dados com base em atual
	if (atual < 0) {
		// uma palavra é suficiente para 64 posições
		peoesPretos.Count(1).Reset(0);
		for (int i = 0; i < 64; i++)
			peoesPretos.SetBit(i, tabuleiro[i] == 'p');
		tomaPecas.Count(pecas.Count());
		if (Parametro(NIVEL_DEBUG) >= COMPLETO) {
			// validar mapa de bits
			Debug(false);
			Debug(peoesPretos);
		}
	}
	else
		peoesPretos.SetBit(atual, false); // casa atual não tem peão
	// atualizar apenas peças que tomem esta casa 
	for (int i = 0; i < pecas.Count(); i++) {
		if (atual < 0 || tomaPecas[i].GetBit(atual)) {
			TVector<int> posicoes = { pecas[i] }; // posição inicial
			char peca = tabuleiro[pecas[i]];
			tomaPecas[i].Count(1).Reset(0);
			for (int j = 0; j < posicoes.Count(); j++)
				// considerar que B+T podem saltar outras peças (podem-se mover e permitir seguir)
				for (auto mov : Movimentos(posicoes[j], peca, true))
					// fazer movimento se for tomar um peão preto e não estar já na lista
					if (!tomaPecas[i].GetBit(mov) && tabuleiro[mov] == 'p') {
						tomaPecas[i].SetBit(mov, true); // peão pode ser tomado
						posicoes += mov; // processar desta posição
					}
		}
		tomaveis |= tomaPecas[i];
		if (atual < 0 && Parametro(NIVEL_DEBUG) >= COMPLETO) {
			Debug(tomaPecas[i]);
			printf(" %c em %d", tabuleiro[pecas[i]], pecas[i]);
		}
	}
	if (peoesPretos != tomaveis) {
		if (Parametro(NIVEL_DEBUG) >= COMPLETO) {
			// validar estado cortado
			Debug();
			printf(" impossível");
			Debug(false);
			Debug(peoesPretos);
			printf(" peões a tomar");
			Debug(tomaveis);
			printf(" peões tomáveis");
		}
		return false;
	}
	return true;
}

TString CToma::Acao(TNo sucessor) {
	int atual = ((CToma*)sucessor)->atual;
	return TString().printf("%c%d", atual % 8 + 'a', atual / 8 + 1);
	return "Inv";
}

void CToma::Debug(bool completo)
{
	if (!completo) {
		NovaLinha();
		printf("%s %d", *tabuleiro, atual);
		return;
	}
	NovaLinha();
	printf("  ＡＢＣＤＥＦＧＨ");
	for (int i = 7; i >= 0; i--) {
		NovaLinha();
		printf("%d ", i + 1);
		for (int j = 0; j < 8; j++) {
			// destacar a posição atual 
			if (atual == j + i * 8)
				printf(COR_ATIVO);
			switch (tabuleiro[j + i * 8]) {
			case 'C': printf("%2s", "♘ "); break; // cavalo branco
			case 'B': printf("%2s", "♗ "); break; // bispo branco
			case 'T': printf("%2s", "♖ "); break; // torre branca
			case 'D': printf("%2s", "♕ "); break; // dama branca
			case 'P': printf("%2s", "♙ "); break; // peão branco
			case 'R': printf("%2s", "♔ "); break; // rei branco
			case 'p': printf("%2s", "♟ "); break; // peão preto
			default: printf("%s", ((i + j) % 2 ? "  " : "::")); break; // "▒▒" "::" "░░"
			}
			if (atual == j + i * 8)
				printf(COR_RESET);
		}
		printf(" %d", i + 1);
	}
	NovaLinha();
	printf("  ＡＢＣＤＥＦＧＨ");
}

// mostrar os bits para debug
void CToma::Debug(TBits& bits) {
	NovaLinha();
	for (int i = 0; i < 64; i++)
		printf("%c", bits.GetBit(i) ? '1' : '0');
}


void CToma::Codifica(TBits& estado)
{
	TProcuraConstrutiva::Codifica(estado);
	// codificar números de 3 bits 
	for (int i = 0, index = 0; i < 64; i++, index += 3)
		estado.SetBits(Codigo(tabuleiro[i]), index, 3);
	estado.SetBits(atual + 1, 192, 7); // codificar posição atual, -1 vira 0
}

