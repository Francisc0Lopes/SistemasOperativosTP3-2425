### 1. Indique, para cada uma das afirmações, se a utilização de múltiplas tarefas, num programa a executar-se num sistema operativo UNIX, é uma razão válida.

| Afirmação | Resposta |
|---|---|
| Para ter a execução de dois troços de código concorrentes com espaços de endereçamento separados num mesmo processo. | Não |
| Para poder realizar operações I/O em simultâneo com outras operações num mesmo processo. | Sim |
| Para poder executar dois programas (ficheiros executáveis) diferentes em concorrência e de uma forma mais rápida. | Não |
| Para dividir o processamento por ações concorrentes de forma a maximizar a utilização de toda a capacidade de processamento do hardware | Sim |

### 2. Considere que a função handleClient() processa uma ligação com o cliente indique se as afirmações seguintes são verdadeiras ou falsas

| Afirmação | Resposta |
|---|---|
| Servidor disponível através de um socket no domínio internet atendendo múltiplos clientes em concorrência. | Verdadeiro (segundo código com threads) |
| Servidor disponível através de um socket no domínio internet atendendo múltiplos clientes em sequência. | Verdadeiro (primeiro código sem threads) |
| Servidor disponível através de um socket no domínio internet atendendo múltiplos clientes em sequência. | Verdadeiro (primeiro código sem threads) |