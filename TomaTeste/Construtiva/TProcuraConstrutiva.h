#pragma once
#include "../TProcuraConstrutiva.h"

enum EParametrosToma {
	TIPO_HEUR = PARAMETROS_CONSTRUTIVA, // tipo de heurística em utilização
	PRECALCULAR // precalcular o mais possível
};

/**
 * @brief Representa um estado do problema Toma
 *
 */
class CToma :
	public TProcuraConstrutiva
{
public:
	CToma(void);
	~CToma(void);

	// estrutura de dados: peças num tabuleiro
	TString tabuleiro;
	int atual = -1; // posição da última ação
	TVector<int> pecas; // posição de peças que podem tomar peões
	int ultimaPeca; // posição da peça de onde o rei vem (não vai querer ter saído para voltar lá)
	TBits peoesPretos; // posições dos peões pretos (binário) - Possivel()
	TVector<TBits> tomaPecas; // posições dos peões tomáveis por cada peça (binário) - Possivel()


	// metodos virtuais redefinidos de TProcuraConstrutiva

	TProcuraConstrutiva* Duplicar(void);
	void Copiar(TProcuraConstrutiva* objecto) {
		TProcuraConstrutiva::Copiar(objecto);
		CToma* obj = (CToma*)objecto;
		tabuleiro = obj->tabuleiro;
		atual = obj->atual;
		pecas = obj->pecas;
		ultimaPeca = obj->ultimaPeca;
		if (Parametro(TIPO_HEUR) > 3) {
			peoesPretos = obj->peoesPretos;
			tomaPecas = obj->tomaPecas;
		}
	}
	void Inicializar(void);
	void Gravar(void);
	void ResetParametros();
	void Sucessores(TVector<TNo>& sucessores);
	bool SolucaoCompleta(void);
	void Debug(bool completo = true) override;
	TString Acao(TNo sucessor);
	bool Distinto(TNo estado);

	// estimativa até limpar todos os peõe
	int Heuristica(void);

	void Codifica(TBits& estado);

	void LimparEstatisticas() { TProcuraConstrutiva::LimparEstatisticas(); Possivel(); }

private:
	int Codigo(char peca) {
		switch (peca) {
		case 'C': return 1; // Cavalo
		case 'B': return 2; // Bispo
		case 'T': return 3; // Torre
		case 'D': return 4; // Dama
		case 'p': return 5; // Peão preto
		case 'P': return 6; // Peão branco
		case 'R': return 7; // Rei branco
		default: return 0;  // Vazio ou peça desconhecida
		}
	}

	TVector<int> Movimentos(int pos, char peca, bool saltaPecas = false);

	// atualiza estrutura binária e verifica viabilidade do estado atual
	// atualiza os dados com base em atual
	bool Possivel();

	// gerador de instâncias
	void Gerador(int id);

	// mostrar os bits para debug
	void Debug(TBits &bits);
};

