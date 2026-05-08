Para ignorar mudanças em um arquivo que já está sendo rastreado pelo Git, você tem algumas opções:

1. git update-index --skip-worktree (mais robusto)

```bash
git update-index --skip-worktree data/credentials.enc
```

Essa opção é mais apropriada quando você quer fazer mudanças locais permanentes que não devem ser commitadas. É mais resistente a operações como git pull.
Para desfazer:

```bash
git update-index --no-skip-worktree data/credentials.enc
```

Para listar arquivos marcados assim:

```bash
git ls-files -v | grep '^S'
```

**--skip-worktree**: Para arquivos que você vai modificar localmente mas não quer commitar (como configs locais)
