from utils import ler_instancia
from solver import resolver

tabuleiro = ler_instancia("instancias/instancia_1.txt")

solucao = resolver(tabuleiro)

print("Solução:", solucao)
print("Formato final:", " ".join(solucao))