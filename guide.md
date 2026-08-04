# Codexion — Guide : des dongles jusqu'à la fin du projet

> Ce guide part exactement d'où tu en es : `t_arguments` (avec `dongle`,
> `burnout`, `compile`, `debug`, `refactor`, `coders`, `compiles`,
> `scheduler`, `start_time`, `print_lock`), un `t_coder` avec `id`, `thread`,
> `args`, et une boucle `coder_routine` qui tourne `compiles` fois. On garde
> **tes noms de champs actuels**, pas ceux d'un squelette théorique — chaque
> étape s'ajoute directement à ce que tu as déjà écrit et testé.

---

## Où tu en es (rappel)

```c
/* coders.h — état actuel */
typedef struct s_arguments
{
	int				dongle;       // dongle_cooldown
	int				burnout;      // time_to_burnout
	int				compile;      // time_to_compile
	int				debug;        // time_to_debug
	int				refactor;     // time_to_refactor
	int				coders;       // n_coders
	int				compiles;     // compiles_required
	int				scheduler;    // 1 = fifo, 2 = edf
	struct timeval	start_time;
	pthread_mutex_t	print_lock;
}	t_arguments;

typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	t_arguments	*args;
}	t_coder;
```

Ce qui marche déjà : parsing strict, création/join des threads, logs avec
timestamp protégés par `print_lock`, boucle qui répète `compiles` fois.

Ce qui manque : les dongles, l'anti-deadlock, le cooldown, le vrai
scheduler fifo/edf, le monitor de burnout, l'arrêt **synchronisé** de tous
les coders, le cleanup complet, et le README.

---

## Étape 4 — Les dongles, version naïve (comprendre le mécanisme)

On commence volontairement par une version **simple**, pas encore
anti-deadlock, pour bien voir la mécanique de base avant d'ajouter la
complexité qui la rend sûre.

### a) Structures dans `coders.h`

`t_dongle` doit être déclarée **avant** `t_coder` (qui la référence via
`left`/`right`), et **avant** `t_arguments` (qui contient le tableau).

```c
#ifndef CODERS_H
# define CODERS_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>

typedef struct s_dongle
{
	int				id;
	int				available;
	pthread_mutex_t	lock;
}	t_dongle;

typedef struct s_arguments
{
	int				dongle;         // dongle_cooldown (en ms)
	int				burnout;
	int				compile;
	int				debug;
	int				refactor;
	int				coders;
	int				compiles;
	int				scheduler;
	struct timeval	start_time;
	pthread_mutex_t	print_lock;
	t_dongle		*dongles;       // tableau de `coders` dongles
}	t_arguments;

typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	t_arguments	*args;
	t_dongle	*left;
	t_dongle	*right;
}	t_coder;

int		arguments_validator(char **argv, t_arguments *arguments);
int		ft_strict_atoi(const char *s, long *out);
long	ms_since(struct timeval *ref);
void	log_state(t_arguments *args, int coder_id, const char *msg);
void	dongle_init(t_dongle *d, int id);
void	dongle_destroy(t_dongle *d);
void	dongle_acquire(t_dongle *d);
void	dongle_release(t_dongle *d);
void	*coder_routine(void *args);

#endif
```

**Pourquoi cet ordre exact** : `t_dongle` en premier (aucune dépendance),
`t_arguments` ensuite (contient un `t_dongle *`, ça compile même si
`t_dongle` n'est pas encore *complètement* connue à ce stade — un pointeur
suffit), `t_coder` en dernier (référence `t_arguments *` et `t_dongle *`,
les deux déjà connues).

### b) `dongle.c` — la version simple

```c
#include "coders.h"

void	dongle_init(t_dongle *d, int id)
{
	d->id = id;
	d->available = 1;
	pthread_mutex_init(&d->lock, NULL);
}

void	dongle_destroy(t_dongle *d)
{
	pthread_mutex_destroy(&d->lock);
}

/* Version naïve : attend en boucle (polling) que le dongle soit libre.
** Fonctionne, mais gaspille du CPU — on améliorera ça à l'étape 8 avec
** une pthread_cond_t. Pour l'instant, l'objectif est juste de comprendre
** le principe lock/vérifie/prends/unlock. */
void	dongle_acquire(t_dongle *d)
{
	int	taken;

	taken = 0;
	while (!taken)
	{
		pthread_mutex_lock(&d->lock);
		if (d->available)
		{
			d->available = 0;
			taken = 1;
		}
		pthread_mutex_unlock(&d->lock);
		if (!taken)
			usleep(500);   // évite de spammer le CPU en boucle serrée
	}
}

void	dongle_release(t_dongle *d)
{
	pthread_mutex_lock(&d->lock);
	d->available = 1;
	pthread_mutex_unlock(&d->lock);
}
```

**Décortiquons `dongle_acquire`** :
- On boucle tant qu'on n'a pas réussi à prendre le dongle (`taken == 0`).
- À chaque tour : on verrouille le mutex, on regarde si `available == 1`,
  si oui on le prend (`available = 0`, `taken = 1`), puis on déverrouille
  **avant** de sortir de la boucle.
- Si on n'a pas réussi, on attend un petit peu (`usleep(500)` = 0.5ms) avant
  de réessayer — sinon la boucle tournerait des millions de fois par
  seconde pour rien, ce qui gaspille du CPU inutilement (c'est ce qu'on
  appelle du "polling actif" ; on le remplacera plus tard par un mécanisme
  qui *dort vraiment* jusqu'à être réveillé).

### c) Branche ça dans `main.c` : crée le tableau de dongles

```c
if (!arguments_validator(argv, &arguments))
{
	fprintf(stdout, "Invalid Arguments\n");
	return (0);
}
gettimeofday(&arguments.start_time, NULL);
pthread_mutex_init(&arguments.print_lock, NULL);

arguments.dongles = malloc(sizeof(t_dongle) * arguments.coders);
if (!arguments.dongles)
	return (1);
i = 0;
while (i < arguments.coders)
{
	dongle_init(&arguments.dongles[i], i);
	i++;
}

coders = malloc(sizeof(t_coder) * arguments.coders);
if (!coders)
	return (1);
i = 0;
while (i < arguments.coders)
{
	coders[i].id = i + 1;
	coders[i].args = &arguments;
	coders[i].left = &arguments.dongles[i];
	coders[i].right = &arguments.dongles[(i + 1) % arguments.coders];
	pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
	i++;
}
```

**Le cercle** : `left = dongles[i]`, `right = dongles[(i+1) % n]`. Le `%
n_coders` referme le cercle — le dernier coder (`i = n-1`) a pour `right`
le dongle `0`, celui du tout premier coder. C'est ça qui crée la structure
circulaire décrite dans le sujet.

### d) Utilise les dongles dans `coder.c`

```c
void	*coder_routine(void *args)
{
	t_coder	*coder;
	int		i;

	coder = (t_coder *)args;
	i = 0;
	while (i < coder->args->compiles)
	{
		dongle_acquire(coder->left);
		log_state(coder->args, coder->id, "has taken a dongle");
		dongle_acquire(coder->right);
		log_state(coder->args, coder->id, "has taken a dongle");
		log_state(coder->args, coder->id, "is compiling");
		usleep(coder->args->compile * 1000);
		dongle_release(coder->left);
		dongle_release(coder->right);
		log_state(coder->args, coder->id, "is debugging");
		usleep(coder->args->debug * 1000);
		log_state(coder->args, coder->id, "is refactoring");
		usleep(coder->args->refactor * 1000);
		i++;
	}
	return (NULL);
}
```

### e) N'oublie pas de nettoyer, à la fin de `main.c`

```c
i = 0;
while (i < arguments.coders)
{
	pthread_join(coders[i].thread, NULL);
	i++;
}
i = 0;
while (i < arguments.coders)
{
	dongle_destroy(&arguments.dongles[i]);
	i++;
}
pthread_mutex_destroy(&arguments.print_lock);
free(arguments.dongles);
free(coders);
```

### f) Teste avec peu de coders d'abord

```bash
make re
./codexion 2 4000 200 200 200 3 100 fifo
```

**⚠️ Ce que tu risques de voir avec plus de coders** : avec 2 coders ça
marche presque toujours (un seul dongle partagé entre les deux, en plus du
2ème dongle — avec 2 coders il n'y a que 2 dongles, chacun en a un des
deux mêmes). Teste maintenant avec **4 ou 5 coders**, et relance plusieurs
fois :

```bash
./codexion 5 4000 200 200 200 3 100 fifo
```

Il est possible que ça **bloque** (plus aucune ligne ne s'affiche, le
programme ne se termine jamais). C'est **volontaire** à ce stade — c'est le
deadlock classique du sujet, que l'étape suivante va corriger.

---

## Étape 5 — Corriger le deadlock : ordre total d'acquisition

### Le problème exact

Dans `coder_routine`, tu fais toujours `dongle_acquire(coder->left)` **puis**
`dongle_acquire(coder->right)`. Si tous les coders démarrent en même temps,
chacun réussit à prendre son dongle gauche, puis attend indéfiniment son
dongle droit — qui est le dongle gauche du voisin, lui-même bloqué en train
d'attendre. Cycle fermé, personne n'avance : **deadlock circulaire**.

### La solution : toujours prendre le dongle de plus petit `id` en premier

Modifie uniquement `coder_routine` dans `coder.c` :

```c
void	*coder_routine(void *args)
{
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;
	int			i;

	coder = (t_coder *)args;
	if (coder->left->id < coder->right->id)
	{
		first = coder->left;
		second = coder->right;
	}
	else
	{
		first = coder->right;
		second = coder->left;
	}
	i = 0;
	while (i < coder->args->compiles)
	{
		dongle_acquire(first);
		log_state(coder->args, coder->id, "has taken a dongle");
		dongle_acquire(second);
		log_state(coder->args, coder->id, "has taken a dongle");
		log_state(coder->args, coder->id, "is compiling");
		usleep(coder->args->compile * 1000);
		dongle_release(first);
		dongle_release(second);
		log_state(coder->args, coder->id, "is debugging");
		usleep(coder->args->debug * 1000);
		log_state(coder->args, coder->id, "is refactoring");
		usleep(coder->args->refactor * 1000);
		i++;
	}
	return (NULL);
}
```

**Pourquoi ça marche** : avec un ordre total (tout le monde respecte "le
plus petit id d'abord"), il ne peut plus exister de cycle d'attente. Prends
un exemple à 3 coders/dongles (id 0, 1, 2) :
- Coder A a dongles {0, 1} → prend 0 en premier
- Coder B a dongles {1, 2} → prend 1 en premier
- Coder C a dongles {2, 0} → prend **0** en premier (pas 2, car 0 < 2)

Le dongle `0` est donc systématiquement disputé en premier par A et C.
L'un des deux gagne, prend ensuite son deuxième dongle sans obstacle
(puisque personne ne peut "sauter" par-dessus l'ordre), compile, relâche —
et débloque la situation pour les autres. Il n'y a plus de scénario où
*tout le monde* attend en cercle fermé.

### Teste à nouveau, avec beaucoup de coders

```bash
make re
./codexion 8 4000 200 200 200 5 100 fifo
```

Ça doit maintenant se terminer proprement, quel que soit le nombre de
coders. Relance plusieurs fois pour être sûr qu'il n'y a jamais de blocage.

---

## Étape 6 — Arrêt synchronisé : le flag `stop` partagé

### Le problème actuel

Chaque coder boucle `compiles` fois **de son côté**, indépendamment des
autres. Le sujet veut un arrêt **global** : la simulation continue tant que
**au moins un** coder n'a pas atteint `compiles_required`, et s'arrête dès
que **tous** l'ont atteint (ou dès qu'un burnout survient — étape 9).

### a) Ajoute à `t_arguments`

```c
typedef struct s_arguments
{
	...
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	count_lock;
	int				stop;
}	t_arguments;
```

### b) Ajoute un compteur individuel à `t_coder`

```c
typedef struct s_coder
{
	int			id;
	pthread_t	thread;
	t_arguments	*args;
	t_dongle	*left;
	t_dongle	*right;
	int			compiles_done;
}	t_coder;
```

### c) Prototypes dans `coders.h`

```c
int		is_stopped(t_arguments *args);
void	request_stop(t_arguments *args);
void	register_compile(t_arguments *args, t_coder *coder, t_coder *coders);
```

### d) Crée `state.c`

```c
#include "coders.h"

int	is_stopped(t_arguments *args)
{
	int	v;

	pthread_mutex_lock(&args->stop_lock);
	v = args->stop;
	pthread_mutex_unlock(&args->stop_lock);
	return (v);
}

void	request_stop(t_arguments *args)
{
	pthread_mutex_lock(&args->stop_lock);
	args->stop = 1;
	pthread_mutex_unlock(&args->stop_lock);
}

static int	all_done(t_arguments *args, t_coder *coders)
{
	int	i;

	i = 0;
	while (i < args->coders)
	{
		if (coders[i].compiles_done < args->compiles)
			return (0);
		i++;
	}
	return (1);
}

void	register_compile(t_arguments *args, t_coder *coder, t_coder *coders)
{
	int	finished;

	pthread_mutex_lock(&args->count_lock);
	coder->compiles_done++;
	finished = all_done(args, coders);
	pthread_mutex_unlock(&args->count_lock);
	if (finished)
		request_stop(args);
}
```

**Pourquoi `register_compile` a besoin de `coders` (le tableau complet)** :
pour vérifier "est-ce que TOUS les coders ont fini", il faut regarder le
compteur de chacun, pas juste celui du coder qui vient de terminer.

### e) Modifie `t_coder` pour qu'il connaisse le tableau complet

Le plus simple : ajoute un pointeur vers le tableau lui-même dans
`t_arguments`, plutôt que de le passer partout en paramètre séparé.

```c
typedef struct s_arguments
{
	...
	struct s_coder	*coder_list;   // le tableau de coders, accessible via args
}	t_arguments;
```

### f) `coder.c` — remplace la boucle `while (i < compiles)` par `while (!is_stopped(...))`

```c
void	*coder_routine(void *args)
{
	t_coder		*coder;
	t_dongle	*first;
	t_dongle	*second;

	coder = (t_coder *)args;
	if (coder->left->id < coder->right->id)
	{
		first = coder->left;
		second = coder->right;
	}
	else
	{
		first = coder->right;
		second = coder->left;
	}
	while (!is_stopped(coder->args))
	{
		dongle_acquire(first);
		log_state(coder->args, coder->id, "has taken a dongle");
		dongle_acquire(second);
		log_state(coder->args, coder->id, "has taken a dongle");
		log_state(coder->args, coder->id, "is compiling");
		usleep(coder->args->compile * 1000);
		dongle_release(first);
		dongle_release(second);
		register_compile(coder->args, coder, coder->args->coder_list);
		if (is_stopped(coder->args))
			break ;
		log_state(coder->args, coder->id, "is debugging");
		usleep(coder->args->debug * 1000);
		if (is_stopped(coder->args))
			break ;
		log_state(coder->args, coder->id, "is refactoring");
		usleep(coder->args->refactor * 1000);
	}
	return (NULL);
}
```

**Pourquoi 3 vérifications de `is_stopped` (pas juste au début du `while`)**
: si le dernier coder termine sa compilation et déclenche l'arrêt global
**pendant** que d'autres sont en train de debug/refactor, ces derniers
doivent pouvoir s'arrêter **entre** les phases plutôt que d'attendre le
prochain tour de boucle complet — sinon la fin du programme serait retardée
inutilement.

### g) `main.c` — branche `coder_list` et initialise les nouveaux mutex

```c
gettimeofday(&arguments.start_time, NULL);
pthread_mutex_init(&arguments.print_lock, NULL);
pthread_mutex_init(&arguments.stop_lock, NULL);
pthread_mutex_init(&arguments.count_lock, NULL);
arguments.stop = 0;

arguments.dongles = malloc(sizeof(t_dongle) * arguments.coders);
...
coders = malloc(sizeof(t_coder) * arguments.coders);
arguments.coder_list = coders;   // <- important : APRÈS avoir alloué `coders`

i = 0;
while (i < arguments.coders)
{
	coders[i].id = i + 1;
	coders[i].args = &arguments;
	coders[i].left = &arguments.dongles[i];
	coders[i].right = &arguments.dongles[(i + 1) % arguments.coders];
	coders[i].compiles_done = 0;
	pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
	i++;
}
```

**Attention à l'ordre** : `arguments.coder_list = coders;` doit se faire
**après** le `malloc` du tableau `coders`, sinon tu assignerais un pointeur
non initialisé.

### h) Nettoyage supplémentaire dans `main.c`

```c
pthread_mutex_destroy(&arguments.print_lock);
pthread_mutex_destroy(&arguments.stop_lock);
pthread_mutex_destroy(&arguments.count_lock);
```

### Teste

```bash
make re
./codexion 4 4000 200 200 200 3 100 fifo
```

Le programme doit se terminer dès que **tous** les coders ont atteint 3
compilations — pas chacun de son côté comme avant.

---

## Étape 7 — Le cooldown

### Le principe

Après avoir relâché un dongle, personne (même le coder qui vient de le
relâcher) ne peut le reprendre avant `dongle` millisecondes (ton champ
`arguments.dongle`, c'est le `dongle_cooldown` du sujet).

### a) Ajoute `free_since` à `t_dongle`

```c
typedef struct s_dongle
{
	int				id;
	int				available;
	struct timeval	free_since;
	pthread_mutex_t	lock;
}	t_dongle;
```

### b) Modifie `dongle_init` pour que le premier `acquire` ne soit pas bloqué par un faux cooldown

```c
void	dongle_init(t_dongle *d, int id)
{
	d->id = id;
	d->available = 1;
	d->free_since.tv_sec = 0;    // date très ancienne : cooldown déjà "passé"
	d->free_since.tv_usec = 0;
	pthread_mutex_init(&d->lock, NULL);
}
```

### c) Modifie `dongle_release` pour enregistrer le moment de la libération

```c
void	dongle_release(t_dongle *d)
{
	pthread_mutex_lock(&d->lock);
	d->available = 1;
	gettimeofday(&d->free_since, NULL);
	pthread_mutex_unlock(&d->lock);
}
```

### d) Modifie `dongle_acquire` pour vérifier le cooldown

Comme `dongle_acquire` ne connaissait jusqu'ici que le dongle lui-même, il
faut lui donner accès à `arguments.dongle` (la durée du cooldown). Change
sa signature :

```c
void	dongle_acquire(t_dongle *d, int cooldown_ms)
{
	int	ready;

	ready = 0;
	while (!ready)
	{
		pthread_mutex_lock(&d->lock);
		if (d->available && ms_since(&d->free_since) >= cooldown_ms)
		{
			d->available = 0;
			ready = 1;
		}
		pthread_mutex_unlock(&d->lock);
		if (!ready)
			usleep(500);
	}
}
```

N'oublie pas de mettre à jour le prototype dans `coders.h` :

```c
void	dongle_acquire(t_dongle *d, int cooldown_ms);
```

### e) Mets à jour les appels dans `coder.c`

```c
dongle_acquire(first, coder->args->dongle);
log_state(coder->args, coder->id, "has taken a dongle");
dongle_acquire(second, coder->args->dongle);
log_state(coder->args, coder->id, "has taken a dongle");
```

### Teste avec un cooldown élevé pour bien voir l'effet

```bash
make re
./codexion 3 4000 200 200 200 3 1000 fifo
```

Avec `dongle_cooldown = 1000`ms, tu devrais voir des délais visibles entre
la libération d'un dongle et sa réutilisation, dans les timestamps des logs.

---

## Étape 8 — Le vrai scheduler FIFO / EDF

### Pourquoi c'est nécessaire

Actuellement, `dongle_acquire` fait du "premier qui gagne la course au
`lock`", ce qui ne garantit **aucun ordre précis**. Le sujet veut un vrai
ordre : fifo (premier arrivé, premier servi) ou edf (le plus proche de son
burnout passe en premier).

### a) Ajoute une structure de requête et une file par dongle

```c
typedef struct s_request
{
	int					coder_id;
	long				priority_key;
	long				seq;
	struct s_request	*next;
}	t_request;

typedef struct s_dongle
{
	int				id;
	int				available;
	struct timeval	free_since;
	long			next_seq;
	t_request		*queue;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
}	t_dongle;
```

`t_request` doit être déclarée **avant** `t_dongle`.

### b) `queue.c` — la file triée, commune aux deux schedulers

```c
#include "coders.h"

static int	request_lt(t_request *a, t_request *b)
{
	if (a->priority_key != b->priority_key)
		return (a->priority_key < b->priority_key);
	return (a->seq < b->seq);
}

void	request_enqueue(t_dongle *d, t_request *req)
{
	t_request	**cur;

	cur = &d->queue;
	while (*cur && request_lt(*cur, req))
		cur = &(*cur)->next;
	req->next = *cur;
	*cur = req;
}

int	request_is_front(t_dongle *d, int coder_id)
{
	return (d->queue != NULL && d->queue->coder_id == coder_id);
}

void	request_remove_front(t_dongle *d)
{
	if (d->queue)
		d->queue = d->queue->next;
}
```

### c) Réécris `dongle_acquire` avec la vraie priorité + une vraie attente (plus de polling)

Il faut maintenant que `dongle_acquire` connaisse le coder demandeur (pour
son id et, en edf, sa deadline), pas juste le dongle :

```c
static long	compute_priority_key(t_arguments *args, t_coder *coder)
{
	if (args->scheduler == 2)   // edf
		return (ms_since(&coder->last_compile_start) * -1 + args->burnout);
	return (ms_since(&args->start_time));   // fifo : "maintenant" = arrivée
}

int	dongle_acquire(t_dongle *d, t_arguments *args, t_coder *coder)
{
	t_request	req;

	pthread_mutex_lock(&d->lock);
	req.coder_id = coder->id;
	req.priority_key = compute_priority_key(args, coder);
	req.seq = d->next_seq++;
	req.next = NULL;
	request_enqueue(d, &req);
	while (!is_stopped(args) && !(d->available
			&& ms_since(&d->free_since) >= args->dongle
			&& request_is_front(d, coder->id)))
		pthread_cond_wait(&d->cond, &d->lock);
	if (is_stopped(args))
	{
		pthread_mutex_unlock(&d->lock);
		return (0);
	}
	d->available = 0;
	request_remove_front(d);
	pthread_mutex_unlock(&d->lock);
	return (1);
}
```

**Changement important** : `dongle_acquire` retourne maintenant un `int` (1
= réussi, 0 = abandonné car la simulation s'est arrêtée pendant l'attente).
`coder_routine` doit gérer ce cas — voir §9 ci-dessous, où ça devient
indispensable avec le monitor.

**Pourquoi `compute_priority_key` en edf est un peu particulier ici** :
comme `t_coder` n'a pas encore de vrai champ `last_compile_start` protégé
(on l'ajoute à l'étape 9 avec le monitor), cette formule est provisoire. On
la corrige proprement à l'étape suivante — retiens juste le principe : la
clé edf doit représenter "à quel point ce coder est proche de son
burnout", calculée **une seule fois** au moment de la requête.

### d) `dongle_release` doit réveiller les threads en attente

```c
void	dongle_release(t_dongle *d)
{
	pthread_mutex_lock(&d->lock);
	d->available = 1;
	gettimeofday(&d->free_since, NULL);
	pthread_cond_broadcast(&d->cond);
	pthread_mutex_unlock(&d->lock);
}
```

### e) N'oublie pas d'initialiser/détruire la cond dans `dongle_init`/`dongle_destroy`

```c
void	dongle_init(t_dongle *d, int id)
{
	d->id = id;
	d->available = 1;
	d->free_since.tv_sec = 0;
	d->free_since.tv_usec = 0;
	d->next_seq = 0;
	d->queue = NULL;
	pthread_mutex_init(&d->lock, NULL);
	pthread_cond_init(&d->cond, NULL);
}

void	dongle_destroy(t_dongle *d)
{
	pthread_mutex_destroy(&d->lock);
	pthread_cond_destroy(&d->cond);
}
```

On finalise complètement cette étape juste après avoir ajouté
`last_compile_start` au §9, pour que la clé EDF soit correcte.

---

## Étape 9 — Le monitor : détection précise du burnout

C'est la pièce la plus importante qu'il te manque. Sans elle, ta simulation
ne peut jamais détecter/afficher un burnout.

### a) Ajoute `last_compile_start` à `t_coder`

```c
typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_arguments		*args;
	t_dongle		*left;
	t_dongle		*right;
	int				compiles_done;
	struct timeval	last_compile_start;
}	t_coder;
```

### b) Ajoute à `t_arguments` ce qu'il faut pour le monitor

```c
typedef struct s_arguments
{
	...
	pthread_mutex_t	state_lock;
	pthread_cond_t	state_cond;
	pthread_t		monitor;
}	t_arguments;
```

### c) Corrige maintenant `compute_priority_key` proprement (edf correct)

```c
static long	compute_priority_key(t_arguments *args, t_coder *coder)
{
	long	key;

	if (args->scheduler == 2)
	{
		pthread_mutex_lock(&args->state_lock);
		key = tv_diff_ms(&coder->last_compile_start, &args->start_time)
			+ args->burnout;
		pthread_mutex_unlock(&args->state_lock);
		return (key);
	}
	return (ms_since(&args->start_time));
}
```

Ajoute `tv_diff_ms` dans `utils.c` (utile aussi ailleurs) :

```c
long	tv_diff_ms(struct timeval *later, struct timeval *earlier)
{
	long	sec_diff;
	long	usec_diff;

	sec_diff = later->tv_sec - earlier->tv_sec;
	usec_diff = later->tv_usec - earlier->tv_usec;
	return (sec_diff * 1000 + usec_diff / 1000);
}
```

Et `tv_add_ms` (pour construire une deadline absolue en `timespec`, utilisée
par `pthread_cond_timedwait`) :

```c
void	tv_add_ms(struct timeval *base, long ms, struct timespec *out)
{
	long	total_usec;

	total_usec = (long)base->tv_usec + (ms % 1000) * 1000;
	out->tv_sec = base->tv_sec + ms / 1000 + total_usec / 1000000;
	out->tv_nsec = (total_usec % 1000000) * 1000;
}
```

N'oublie pas les prototypes dans `coders.h` :
```c
long	tv_diff_ms(struct timeval *later, struct timeval *earlier);
void	tv_add_ms(struct timeval *base, long ms, struct timespec *out);
```

### d) `monitor.c`

```c
#include "coders.h"

static void	deadline_of(t_coder *c, long burnout, struct timespec *out)
{
	tv_add_ms(&c->last_compile_start, burnout, out);
}

static int	timespec_lt(struct timespec *a, struct timespec *b)
{
	if (a->tv_sec != b->tv_sec)
		return (a->tv_sec < b->tv_sec);
	return (a->tv_nsec < b->tv_nsec);
}

static int	find_earliest(t_arguments *args, struct timespec *out, int *idx)
{
	int				i;
	struct timespec	ts;
	int				found;

	found = 0;
	i = 0;
	while (i < args->coders)
	{
		deadline_of(&args->coder_list[i], args->burnout, &ts);
		if (!found || timespec_lt(&ts, out))
		{
			*out = ts;
			*idx = i;
			found = 1;
		}
		i++;
	}
	return (found);
}

void	*monitor_routine(void *arg)
{
	t_arguments		*args;
	struct timespec	deadline;
	int				idx;
	int				rc;

	args = (t_arguments *)arg;
	pthread_mutex_lock(&args->state_lock);
	while (!is_stopped(args))
	{
		if (!find_earliest(args, &deadline, &idx))
			break ;
		rc = pthread_cond_timedwait(&args->state_cond, &args->state_lock,
				&deadline);
		if (is_stopped(args))
			break ;
		if (rc == ETIMEDOUT && ms_since(&args->coder_list[idx].last_compile_start)
				>= args->burnout)
		{
			pthread_mutex_unlock(&args->state_lock);
			log_state(args, args->coder_list[idx].id, "burned out");
			request_stop(args);
			pthread_mutex_lock(&args->state_lock);
			break ;
		}
	}
	pthread_mutex_unlock(&args->state_lock);
	return (NULL);
}
```

**Le principe, expliqué** : au lieu de vérifier en boucle serrée (gaspille
du CPU, imprécis), le monitor calcule la deadline la plus proche parmi tous
les coders et **dort dessus** avec `pthread_cond_timedwait`. Il se réveille
soit parce que le temps est écoulé (`ETIMEDOUT`, vrai burnout probable, on
revérifie pour être sûr), soit parce qu'un coder a démarré une nouvelle
compilation entre-temps (`pthread_cond_broadcast` sur `state_cond`, voir
ci-dessous) — dans ce cas on recalcule juste la nouvelle deadline la plus
proche et on continue.

Ajoute `#include <errno.h>` dans `coders.h` pour `ETIMEDOUT`, et le
prototype :
```c
void	*monitor_routine(void *arg);
```

### e) `coder_routine` doit notifier le monitor à chaque nouvelle compilation

Ajoute une petite fonction dans `coder.c` :

```c
static void	mark_compile_start(t_coder *c)
{
	pthread_mutex_lock(&c->args->state_lock);
	gettimeofday(&c->last_compile_start, NULL);
	pthread_cond_broadcast(&c->args->state_cond);
	pthread_mutex_unlock(&c->args->state_lock);
}
```

Et branche-la juste après avoir obtenu les deux dongles :

```c
if (!dongle_acquire(first, coder->args, coder))
    break ;
log_state(coder->args, coder->id, "has taken a dongle");
if (!dongle_acquire(second, coder->args, coder))
{
    dongle_release(first);
    break ;
}
log_state(coder->args, coder->id, "has taken a dongle");
mark_compile_start(coder);
log_state(coder->args, coder->id, "is compiling");
```

### f) `request_stop` doit réveiller TOUT le monde (monitor + coders bloqués sur un dongle)

Sinon, un coder bloqué dans `dongle_acquire` au moment d'un burnout ailleurs
resterait bloqué pour toujours, et `pthread_join` ne reviendrait jamais.
Mets à jour `state.c` :

```c
void	request_stop(t_arguments *args)
{
	int	i;

	pthread_mutex_lock(&args->stop_lock);
	args->stop = 1;
	pthread_mutex_unlock(&args->stop_lock);
	pthread_mutex_lock(&args->state_lock);
	pthread_cond_broadcast(&args->state_cond);
	pthread_mutex_unlock(&args->state_lock);
	i = 0;
	while (i < args->coders)
	{
		pthread_mutex_lock(&args->dongles[i].lock);
		pthread_cond_broadcast(&args->dongles[i].cond);
		pthread_mutex_unlock(&args->dongles[i].lock);
		i++;
	}
}
```

### g) `main.c` — initialise, lance et attend le monitor

```c
gettimeofday(&arguments.start_time, NULL);
pthread_mutex_init(&arguments.print_lock, NULL);
pthread_mutex_init(&arguments.stop_lock, NULL);
pthread_mutex_init(&arguments.count_lock, NULL);
pthread_mutex_init(&arguments.state_lock, NULL);
pthread_cond_init(&arguments.state_cond, NULL);
arguments.stop = 0;
...
i = 0;
while (i < arguments.coders)
{
	coders[i].id = i + 1;
	coders[i].args = &arguments;
	coders[i].left = &arguments.dongles[i];
	coders[i].right = &arguments.dongles[(i + 1) % arguments.coders];
	coders[i].compiles_done = 0;
	coders[i].last_compile_start = arguments.start_time;   // <- important
	i++;
}
pthread_create(&arguments.monitor, NULL, monitor_routine, &arguments);
i = 0;
while (i < arguments.coders)
{
	pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
	i++;
}
i = 0;
while (i < arguments.coders)
{
	pthread_join(coders[i].thread, NULL);
	i++;
}
pthread_join(arguments.monitor, NULL);
```

Nettoyage supplémentaire :
```c
pthread_mutex_destroy(&arguments.state_lock);
pthread_cond_destroy(&arguments.state_cond);
```

### Teste un vrai burnout

Choisis des paramètres où le burnout est inévitable (cooldown trop long par
rapport au temps de burnout) :

```bash
make re
./codexion 4 500 200 200 200 100 400 fifo
```

Tu dois voir un `"X burned out"` apparaître, et le programme se terminer
proprement (pas de blocage). Vérifie que le timestamp du burnout est très
proche de la deadline théorique (`time_to_burnout` après le dernier
`last_compile_start` de ce coder) — l'écart doit être largement sous 10ms.

Teste aussi `n_coders = 1` (un seul dongle, il ne peut jamais en réunir
deux) : il doit maintenant burn out proprement au lieu de bloquer pour
toujours.

```bash
./codexion 1 500 200 200 200 5 100 fifo
```

Teste enfin `edf` avec des paramètres serrés et plusieurs coders, pour voir
que le scheduler protège les coders les plus proches du burnout :

```bash
./codexion 5 1000 200 200 200 10 100 edf
```

---

## Étape 10 — Cleanup complet et vérification finale

### Checklist mémoire/threads

- [ ] Chaque `pthread_create` a bien son `pthread_join` correspondant (tous
      les coders + le monitor)
- [ ] Chaque `pthread_mutex_init` a son `pthread_mutex_destroy`
      (`print_lock`, `stop_lock`, `count_lock`, `state_lock`, et le `lock`
      de **chaque** dongle)
- [ ] Chaque `pthread_cond_init` a son `pthread_cond_destroy`
      (`state_cond`, et le `cond` de **chaque** dongle)
- [ ] Chaque `malloc` a son `free` (`arguments.dongles`, `coders`)
- [ ] Aucun accès à une donnée partagée sans mutex (relis chaque endroit où
      tu touches `stop`, `compiles_done`, `last_compile_start`,
      `available`, `queue`)

### Norme

- [ ] `-Wall -Wextra -Werror -pthread` : zéro warning
- [ ] Fonctions ≤ 25 lignes, ≤ 4 paramètres, un seul niveau de logique par
      fonction — si `coder_routine` est trop longue, découpe-la en
      sous-fonctions statiques (`coder_take_dongles`, `coder_release_all`,
      etc., comme dans les étapes précédentes)
- [ ] `Makefile` avec les règles `$(NAME)`, `all`, `clean`, `fclean`, `re`

### Tests à faire toi-même avant de rendre

```bash
# complétion normale
./codexion 3 4000 200 200 200 5 100 fifo

# stress test, beaucoup de coders
./codexion 10 4000 200 200 200 10 100 fifo
./codexion 10 4000 200 200 200 10 100 edf

# burnout forcé
./codexion 4 500 200 200 200 100 400 fifo

# cas limite n=1
./codexion 1 4000 200 200 200 5 100 fifo

# arguments invalides
./codexion 3 -1 200 200 200 5 100 fifo
./codexion 3 abc 200 200 200 5 100 fifo
./codexion 3 4000 200 200 200 5 100 xyz
```

### Valgrind (indispensable, à faire dans ton environnement local)

```bash
valgrind --leak-check=full --show-leak-kinds=all ./codexion 3 4000 200 200 200 3 100 fifo
valgrind --tool=helgrind ./codexion 3 4000 200 200 200 3 100 fifo
```

Zéro fuite, zéro data race signalée.

---

## Étape 11 — README.md

Sections obligatoires (voir sujet, chapitre VII), dans cet ordre :

```markdown
*This project has been created as part of the 42 curriculum by <ton_login>.*

## Description
Codexion simulates coders sharing dongles in a circular co-working hub...
(explique le problème, le lien avec les dining philosophers, l'objectif)

## Instructions
### Compilation
\`\`\`bash
make
\`\`\`
### Usage
\`\`\`bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
    time_to_refactor number_of_compiles_required dongle_cooldown scheduler
\`\`\`
### Example
\`\`\`bash
./codexion 4 4000 200 200 200 5 100 fifo
\`\`\`

## Resources
- POSIX threads documentation (man pthread_create, pthread_mutex_lock...)
- Dijkstra's Dining Philosophers problem
- AI usage: [décris précisément quelles parties, quelles tâches]

## Blocking cases handled
- Deadlock prevention: total ordering of dongle acquisition (smallest id
  first), breaking Coffman's circular wait condition
- Starvation prevention: fifo/edf priority queue per dongle
- Cooldown handling: timestamp-based, revalidated on every wakeup
- Precise burnout detection: monitor thread using pthread_cond_timedwait
  on the earliest deadline
- Log serialization: single mutex-protected print function

## Thread synchronization mechanisms
- print_lock: serializes all log output
- stop_lock: protects the shared stop flag
- count_lock: protects compiles_done counters
- state_lock + state_cond: protects last_compile_start, wakes the monitor
- one mutex + one cond per dongle: protects availability and the
  fifo/edf request queue
[explique comment chacun prévient une race condition précise, avec un
exemple concret comme dans ce guide]
```

Rédigé entièrement en anglais.

---

## Récapitulatif — dans quel fichier se trouve quoi, à la fin

| Fichier | Contient |
|---|---|
| `coders.h` | Toutes les structures + tous les prototypes |
| `main.c` | Parsing, init, création/join des threads, cleanup |
| `parser.c` | `arguments_validator`, `scheduler_converter` |
| `utils.c` | `ft_strict_atoi`, `ms_since`, `tv_diff_ms`, `tv_add_ms`, `log_state` |
| `dongle.c` | `dongle_init/destroy/acquire/release`, `compute_priority_key` |
| `queue.c` | `request_enqueue`, `request_is_front`, `request_remove_front` |
| `coder.c` | `coder_routine`, `mark_compile_start` |
| `monitor.c` | `monitor_routine`, `find_earliest`, `deadline_of` |
| `state.c` | `is_stopped`, `request_stop`, `register_compile` |

Ce découpage garde chaque fichier centré sur **une seule responsabilité**,
ce qui aide beaucoup en soutenance : tu peux répondre "où est fait X ?" en
une seconde, et chaque fonction reste courte et explicable individuellement.
