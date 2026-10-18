# Cobertura da recompilação

O aviso de trecho não traduzido refere-se a **código de máquina Z80 sem uma tradução nativa compatível naquele banco/endereço**. O texto em português do cartucho é outro assunto. Ocultar o aviso, ignorar a instrução ou seguir executando com estado incorreto não resolveria o problema.

## Geração da revisão 0.10

- 7.562 pares banco/endereço foram observados em execução de referência.
- O fechamento conservador produz 200.067 entradas candidatas. Esse total inclui offsets que possivelmente são dados, não instruções realmente executadas.
- Todos os offsets das combinações de banco/janela observadas são considerados como possíveis entradas, para alcançar saltos indiretos dentro dessas janelas.
- Ramos diretos para a página 2 consideram todos os bancos selecionáveis da ROM pelo mapper Sega.
- Cada função de despacho gerada cobre no máximo 1 KiB de endereços convidados. Isso evita um único switch gigante para o compilador; não muda a sequência de instruções da CPU.
- Os bytes da instrução são conferidos antes de executar a tradução. O build nativo não recorre ao interpretador para esconder uma lacuna.

Ainda não há garantia para código executado em RAM, mapeamentos não observados das demais janelas, operandos cruzando fronteiras de banco ou falhas semânticas do backend. Contar candidatos não mede uma porcentagem de conclusão do jogo.

## Percursos reproduzíveis

| Região | Situação |
|---|---|
| Abertura sem pular | Roteiro de 5.000 quadros |
| Primeira casa e diálogo | Entrada, texto, saída e save no diálogo |
| Primeira fase | Percurso, sala de chefe e chegada à segunda fase |
| Segunda fase e reinício | Exploração determinística até 18.500 quadros; não comprova todos os serviços e saídas |
| Demais fases, segredos e final | Ainda precisam de percursos completos |

Os testes comparam imagem e áudio com o executor de referência e verificam o hash do estado ao restaurar saves em outro processo. Os dois caminhos compartilham partes do runtime; essa comparação não substitui validação com hardware ou outra implementação independente. Consulte os resultados em VALIDATION.md.

## Como reproduzir uma lacuna

Se ocorrer um miss, o aplicativo tenta gravar `coverage-last-route.txt` no diretório de dados, junto de `runtime.log`. O roteiro contém apenas intervalos `PRIMEIRO_QUADRO ULTIMO_QUADRO MASCARA`, inclusivos, a partir do quadro 1; não contém ROM, memória, imagens ou tokens. Somente a falha atualiza esse arquivo. Não há envio automático e os slots de save não são sobrescritos.

Para a ferramenta de desenvolvimento, defina `MONICA_VERIFY_TIMELINE` com o caminho desse arquivo e execute `monica-verify` com a ROM local e um limite de quadros após a falha. A referência pode usar `MONICA_VERIFY_REFERENCE=1` para registrar destinos faltantes em um perfil local. Qualquer alteração posterior deve voltar a passar pelos testes nativos, sem fallback.
