#ifndef INLINE_CHANSERV_H
#define INLINE_CHANSERV_H

typedef void (*cs_cmd_proto)(sourceinfo_t*, int, char**);

static inline unsigned int custom_founder_check(void)
{
	char *p;

	if (chansvs.founder_flags != NULL && (p = strchr(chansvs.founder_flags, 'F')) != NULL)
		return flags_to_bitmask(chansvs.founder_flags, 0);
	else
		return CA_INITIAL & ca_all;
}

static inline bool invert_purpose(sourceinfo_t *si, int parc, char *chan, char **nick, char neg, cs_cmd_proto func)
{
	char *negargar[2];

	if ((*nick)[0] != '+' && (*nick)[0] != '-')
		return false;

	if ((*nick)[1] == '+' || (*nick)[1] == '-' || (*nick)[1] == '\0')
	{
		command_fail(si, fault_badparams, _("Not a valid action: \2%s\2"), *nick);
		return true;
	}

	if ((*nick)[0] == (neg != '+' ? '-' : '+'))
	{
		negargar[0] = chan;
		negargar[1] = *nick + 1;
		(*func)(si, parc, negargar);
		return true;
	}
	else if((*nick)[0] == (neg == '+' ? '-' : '+'))
		(*nick)++;
	return false;
}

struct prefix_action
{
	bool en; /* en for "enable" */
	char nick[0]; /* CAREFUL! */
};

static inline void prefix_action_set(mowgli_list_t *actions, char *nick, bool en)
{
	struct prefix_action *act;
	mowgli_node_t *n;

	MOWGLI_LIST_FOREACH(n, actions->head)
	{
		act = n->data;
		if (!strcmp(act->nick, nick))
		{
			act->en = en;
			return;
		}
	}

	act = smalloc(sizeof(*act) + strlen(nick) + 1);
	act->en = en;
	strcpy(act->nick, nick);
	mowgli_node_add(act, mowgli_node_create(), actions);
}

static inline void prefix_action_set_all(mowgli_list_t *actions, bool dfl, char *nicklist)
{
	char *nick, *strtokctx;
	bool en;

	nick = strtok_r(nicklist, " ", &strtokctx);
	do
	{
		switch (*nick)
		{
		case '-':
			en = false;
			nick++;
			break;
		case '+':
			en = true;
			nick++;
			break;
		default:
			en = dfl;
		}

		prefix_action_set(actions, nick, en);
	} while ((nick = strtok_r(NULL, " ", &strtokctx)) != NULL);
}

static inline void prefix_action_clear(mowgli_list_t *actions)
{
	mowgli_node_t *n, *tn;

	MOWGLI_LIST_FOREACH_SAFE(n, tn, actions->head)
	{
		free(n->data);
		mowgli_node_delete(n, actions);
		mowgli_node_free(n);
	}
}

#endif
