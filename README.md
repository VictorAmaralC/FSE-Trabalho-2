# Projeto 2

## Resumo

<p align="justify">&emsp;&emsp;Este trabalho tem por objetivo a implementação de um sistema (que simula) o controle de um forno para soldagem de placas de circuito impresso (PCBs).</p>

<p align="justify">&emsp;&emsp;No trabalho, o aluno deverá desenvolver o software que efetua o controle de temperatura do forno utilizando dois atuadores para este controle: um resistor de potência de 15 Watts utilizado para aumentar temperatura e; uma ventoinha que puxa o ar externo (temperatura ambiente) para reduzir a temperatura do sistema.</p>

## Compilação

<p align="justify">&emsp;&emsp;Para gerar um arquivo executável, basta utilizar o comando <code>make</code> na raíz do projeto, onde está localizado o arquvio Makefile. Um arquivo executável <code>bin</code> será gerado na pasta bin.</p>

## Uso

<p align="justify">&emsp;&emsp;Ao iniciar o programa, será apresentado um menu ao usuário. As informações de temperatura e valor de acionamento da ventoinha/resistor são atualizadas periodicamente.</p>

<p align="justify">&emsp;&emsp;O menu pode ser controlado utilizando as setas do teclado. Pressione a tecla Enter para selecionar a opção destacada.</p>

* Selecionar a primeira opção apresentará um campo de input pra o usuário. Digite o valor desejado e pressione Enter.
* Selecionar a segunda opção atualizará a temperatura de referência para o potenciômetro.
* Selecionar a terceira opção encerrará as comunicações e o programa.

<p align="justify">&emsp;&emsp;Atente-se ao intervalo de temperatura permitido na primeira opção. Não é possível inserir temperaturas maiores que 100 ºC ou menores que a temperatura ambiente. O programa irá notificar o usuário caso o intervalo não seja respeitado e reverterá para o potenciômetro.</p>

<p align="justify">&emsp;&emsp;O resistor é acionado assim que o valor lido é superior à 0%. Já a ventoinha é acionada somente em valores inferiores à -40%. Os dispositivos nunca são acionados simultaneamente.</p>

### Observações

<p align="justify">&emsp;&emsp;É recomendado maximizar o terminal antes de executar o programa para evitar problemas na apresentação do menu.</p>

### Referências

[Ncurses](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
