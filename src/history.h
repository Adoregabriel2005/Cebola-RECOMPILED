#pragma once
/* Original editorial summaries. Source provenance and research limits are in
   docs/HISTORIA.md. No third-party article or video is bundled here. */
typedef struct {
    const wchar_t *name,*kicker,*body,*source_label,*source_url,*other_label,*other_url;
} HistoryChapter;
static const HistoryChapter history_chapters[]={
{L"A aventura",L"1991 · Master System · Brasil · 1 jogador · 12 fases",
 L"Publicado pela Tec Toy, Mônica no Castelo do Dragão adapta Wonder Boy in Monster Land, da Westone/Sega. O cartucho tem 256 KiB: os ‘2 MEGA’ da capa são megabits, não megabytes. [1]\n\n"
 L"O Capitão Feio quer dominar e poluir o mundo. Mônica enfrenta seu exército de monstros com o Sansão; fracassar significaria virar um monstro de sujeira. Essa é a premissa da contracapa brasileira.\n\n"
 L"A jornada mistura plataformas, combates, ouro e equipamentos. Portas levam a conversas e serviços; a ampulheta pressiona o jogador a avançar. A história e os créditos do cartucho pertencem aos produtores originais.",
 L"[1] Acervo e ficha",L"https://www.smspower.org/Games/MonicaNoCasteloDoDragao-SMS",L"Pesquisa cultural",L"https://doi.org/10.11606/issn.1982-8160.v19i1p173-197"},
{L"Como nasceu a parceria",L"Uma personagem brasileira encontra o catálogo da Sega",
 L"A relação com a Mauricio de Sousa Produções antecede o cartucho: a Estrelinha Mágica já havia virado brinquedo da Tec Toy em 1989. A pesquisa de Emmanoel Ferreira situa essa parceria anterior no contexto da adaptação de 1991. [2]\n\n"
 L"Alexandre Pagano, participante do projeto, conta que a equipe já alterava textos e personagens quando Stefano Arnhold levou a ideia a Maurício de Sousa. Wonder Boy foi escolhido por permitir essa adaptação. [3]\n\n"
 L"Maurício questionou a espada nas mãos de Mônica e propôs o Sansão. Segundo Pagano, a equipe passou então a trabalhar com o estúdio nessa mudança. A adaptação comercial teve autorização da Sega. [2, 3]",
 L"[3] Entrevista Pagano",L"https://qgmaster.blogspot.com/2016/07/entrevista-alexandre-pagano-da-tectoy_31.html",L"[2] Pesquisa / USP",L"https://doi.org/10.11606/issn.1982-8160.v19i1p173-197"},
{L"A oficina de pixels",L"Relatos de quem participou · não um registro completo da produção",
 L"Pagano descreve um trabalho artesanal: desenhos em papel quadriculado, cada quadrado representando um pixel, e animações refeitas quadro a quadro. A engenharia criou ferramentas próprias; o ambiente citado era o DOS. [3]\n\n"
 L"Na mesma entrevista, ele afirma que a Sega não fornecia código, cursos ou software para esse trabalho. O relato explica a importância da engenharia local, mas não fornece uma lista completa de ferramentas e créditos de cada etapa.\n\n"
 L"As reportagens sobre O Resgate tratam da continuação. Não atribuímos automaticamente a este cartucho seus personagens, suas ferramentas específicas ou fotografias de bastidores.",
 L"[3] Relato de produção",L"https://qgmaster.blogspot.com/2016/07/entrevista-alexandre-pagano-da-tectoy_31.html",L"Estudo SBGames",L"https://www.sbgames.org/sbgames2011/proceedings/sbgames/papers/art/full/92184.pdf"},
{L"O que mudou de Wonder Boy",L"Monster Land: arcade 1987 · Master System 1988 · Mônica 1991",
 L"PROTAGONISTA E ARMA · O herói de Wonder Boy dá lugar à Mônica; as espadas tornam-se o Sansão. No inventário, ‘Sword’ vira ‘Coelho’, com níveis de força em vez de nomes de espadas. [4]\n\n"
 L"HISTÓRIA E ITENS · Os textos em português ligam a aventura ao Capitão Feio. Armaduras passam a vestidos e os diálogos ganham contexto local. Não é apenas uma tradução literal. [4]\n\n"
 L"CENÁRIOS E REGRAS · A estrutura das fases, lojas, desafios e grande parte dos gráficos de inimigos e ambientes permanecem da base. A adaptação não redesenha todos os cenários nem troca todo personagem pela Turma da Mônica. [5]",
 L"[4] Comparação / USP",L"https://www.teses.usp.br/teses/disponiveis/8/8160/tde-03122015-131933/publico/2015_RicardoViniciusFerrazDeSouza_VCorr.pdf",L"[5] Estudo SBGames",L"https://www.sbgames.org/sbgames2011/proceedings/sbgames/papers/art/full/92184.pdf"},
{L"Nas páginas das revistas",L"Capa, anúncio e memória de lançamento",
 L"A capa brasileira apresenta Mônica e Sansão cercados por monstros, com a identidade quadriculada do Master System. A contracapa explica a ameaça de poluição do Capitão Feio. Os materiais à direita foram fornecidos para este acervo.\n\n"
 L"O anúncio ‘A primeira aventura da Mônica onde não é o Cebolinha que apanha’ foi identificado por Ricardo V. F. de Souza na Supergame nº 6, janeiro de 1992, página 6. A edição documenta a campanha, sem estabelecer sozinha a data exata do lançamento. [4]\n\n"
 L"O catálogo de cartuchos da Tec Toy preservado pelo SMS Power! também inclui este título. Juntos, catálogo, capa e anúncio mostram como a aventura foi apresentada ao público brasileiro. As datas e a origem de cada peça devem ser preservadas.",
 L"[4] Análise do anúncio",L"https://www.teses.usp.br/teses/disponiveis/8/8160/tde-03122015-131933/publico/2015_RicardoViniciusFerrazDeSouza_VCorr.pdf",L"Catálogo Tec Toy",L"https://www.smspower.org/Scans/TecToy-Catalogue-CartuchosMasterSystem"},
{L"Vendas e legado",L"O que os documentos permitem afirmar",
 L"VENDAS · Não localizamos um total auditável de unidades deste cartucho. Sem fonte e método, não publicamos uma estimativa. Números de consoles vendidos, reedições com jogos na memória e leilões de usados não equivalem a vendas deste título.\n\n"
 L"LEGADO · O jogo é estudado como exemplo de adaptação cultural: uma aventura estrangeira passa a dialogar com personagens conhecidos do público brasileiro. A pesquisa de Ferreira discute essa presença local no catálogo do Master System. [2]\n\n"
 L"A série continuou com O Resgate (1993), baseado em The Dragon’s Trap, e Na Terra dos Monstros (1994), para Mega Drive, baseado em Wonder Boy in Monster World. São outros jogos, com outras alterações. [6]",
 L"[2] Contexto cultural",L"https://doi.org/10.11606/issn.1982-8160.v19i1p173-197",L"[6] Série e adaptações",L"https://www.overloadr.com.br/especiais/reportagens/2016/08/wonder-boy-dragons-trap-monica"},
{L"Fontes e critérios",L"Pesquisa consultada em 18 de outubro de 2026",
 L"[1] SMS Power!: ficha, contracapa e acervo do cartucho. Materiais de época também foram enviados pelo autor do projeto.\n"
 L"[2] Emmanoel Ferreira, MATRIZes 19(1), 2025, p. 173–197: contexto cultural e parceria anterior.\n"
 L"[3] Entrevista de Alexandre Pagano a Rodrigo, QG Master, 31/07/2016: testemunho de produção.\n"
 L"[4] Ricardo Vinícius Ferraz de Souza, USP, 2015: estudo comparativo da localização e publicidade.\n"
 L"[5] Melo, Coimbra, Neves e Porto, SBGames 2011: reaproveitamento e mudança de contexto.\n"
 L"[6] Overloadr, 2016: retrospectiva da série.\n\n"
 L"Depoimentos posteriores são identificados como relatos. Dado ausente permanece ausente. Textos editoriais próprios; imagens e marcas seguem os direitos dos titulares.",
 L"[3] Fonte oral publicada",L"https://qgmaster.blogspot.com/2016/07/entrevista-alexandre-pagano-da-tectoy_31.html",L"[4] Pesquisa completa",L"https://www.teses.usp.br/teses/disponiveis/8/8160/tde-03122015-131933/publico/2015_RicardoViniciusFerrazDeSouza_VCorr.pdf"}
};
#define HISTORY_COUNT ((int)(sizeof history_chapters/sizeof history_chapters[0]))
