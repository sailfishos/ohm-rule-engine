
#ifndef __OHM_FACT_H__
#define __OHM_FACT_H__

#include <glib.h>
#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define OHM_TYPE_STRUCTURE (ohm_structure_get_type ())
#define OHM_STRUCTURE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_STRUCTURE, OhmStructure))
#define OHM_STRUCTURE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_STRUCTURE, OhmStructureClass))
#define OHM_IS_STRUCTURE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_STRUCTURE))
#define OHM_IS_STRUCTURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_STRUCTURE))
#define OHM_STRUCTURE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_STRUCTURE, OhmStructureClass))

typedef struct _OhmStructure OhmStructure;
typedef struct _OhmStructureClass OhmStructureClass;
typedef struct _OhmStructurePrivate OhmStructurePrivate;

#define OHM_TYPE_PATTERN (ohm_pattern_get_type ())
#define OHM_PATTERN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_PATTERN, OhmPattern))
#define OHM_PATTERN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_PATTERN, OhmPatternClass))
#define OHM_IS_PATTERN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_PATTERN))
#define OHM_IS_PATTERN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_PATTERN))
#define OHM_PATTERN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_PATTERN, OhmPatternClass))

typedef struct _OhmPattern OhmPattern;
typedef struct _OhmPatternClass OhmPatternClass;
typedef struct _OhmPatternPrivate OhmPatternPrivate;

#define OHM_PATTERN_TYPE_MATCH (ohm_pattern_match_get_type ())
#define OHM_PATTERN_MATCH(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_PATTERN_TYPE_MATCH, OhmPatternMatch))
#define OHM_PATTERN_MATCH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_PATTERN_TYPE_MATCH, OhmPatternMatchClass))
#define OHM_PATTERN_IS_MATCH(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_PATTERN_TYPE_MATCH))
#define OHM_PATTERN_IS_MATCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_PATTERN_TYPE_MATCH))
#define OHM_PATTERN_MATCH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_PATTERN_TYPE_MATCH, OhmPatternMatchClass))

typedef struct _OhmPatternMatch OhmPatternMatch;
typedef struct _OhmPatternMatchClass OhmPatternMatchClass;
typedef struct _OhmPatternMatchPrivate OhmPatternMatchPrivate;

#define OHM_TYPE_FACT (ohm_fact_get_type ())
#define OHM_FACT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_FACT, OhmFact))
#define OHM_FACT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_FACT, OhmFactClass))
#define OHM_IS_FACT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_FACT))
#define OHM_IS_FACT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_FACT))
#define OHM_FACT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_FACT, OhmFactClass))

typedef struct _OhmFact OhmFact;
typedef struct _OhmFactClass OhmFactClass;
typedef struct _OhmFactPrivate OhmFactPrivate;

#define OHM_TYPE_FACT_STORE (ohm_fact_store_get_type ())
#define OHM_FACT_STORE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_FACT_STORE, OhmFactStore))
#define OHM_FACT_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_FACT_STORE, OhmFactStoreClass))
#define OHM_IS_FACT_STORE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_FACT_STORE))
#define OHM_IS_FACT_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_FACT_STORE))
#define OHM_FACT_STORE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_FACT_STORE, OhmFactStoreClass))

typedef struct _OhmFactStore OhmFactStore;
typedef struct _OhmFactStoreClass OhmFactStoreClass;
typedef struct _OhmFactStorePrivate OhmFactStorePrivate;

#define OHM_FACT_STORE_TYPE_CHANGE_SET (ohm_fact_store_change_set_get_type ())
#define OHM_FACT_STORE_CHANGE_SET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSet))
#define OHM_FACT_STORE_CHANGE_SET_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSetClass))
#define OHM_FACT_STORE_IS_CHANGE_SET(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_FACT_STORE_TYPE_CHANGE_SET))
#define OHM_FACT_STORE_IS_CHANGE_SET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_FACT_STORE_TYPE_CHANGE_SET))
#define OHM_FACT_STORE_CHANGE_SET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_FACT_STORE_TYPE_CHANGE_SET, OhmFactStoreChangeSetClass))

typedef struct _OhmFactStoreChangeSet OhmFactStoreChangeSet;
typedef struct _OhmFactStoreChangeSetClass OhmFactStoreChangeSetClass;
typedef struct _OhmFactStoreChangeSetPrivate OhmFactStoreChangeSetPrivate;

#define OHM_FACT_STORE_TYPE_VIEW (ohm_fact_store_view_get_type ())
#define OHM_FACT_STORE_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_FACT_STORE_TYPE_VIEW, OhmFactStoreView))
#define OHM_FACT_STORE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_FACT_STORE_TYPE_VIEW, OhmFactStoreViewClass))
#define OHM_FACT_STORE_IS_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_FACT_STORE_TYPE_VIEW))
#define OHM_FACT_STORE_IS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_FACT_STORE_TYPE_VIEW))
#define OHM_FACT_STORE_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_FACT_STORE_TYPE_VIEW, OhmFactStoreViewClass))

typedef struct _OhmFactStoreView OhmFactStoreView;
typedef struct _OhmFactStoreViewClass OhmFactStoreViewClass;
typedef struct _OhmFactStoreViewPrivate OhmFactStoreViewPrivate;

#define OHM_FACT_STORE_TYPE_EVENT (ohm_fact_store_event_get_type ())

#define OHM_TYPE_RULE (ohm_rule_get_type ())
#define OHM_RULE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), OHM_TYPE_RULE, OhmRule))
#define OHM_RULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), OHM_TYPE_RULE, OhmRuleClass))
#define OHM_IS_RULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OHM_TYPE_RULE))
#define OHM_IS_RULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), OHM_TYPE_RULE))
#define OHM_RULE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), OHM_TYPE_RULE, OhmRuleClass))

typedef struct _OhmRule OhmRule;
typedef struct _OhmRuleClass OhmRuleClass;
typedef struct _OhmRulePrivate OhmRulePrivate;

struct _OhmStructure {
	GObject parent_instance;
	OhmStructurePrivate * priv;
};
struct _OhmStructureClass {
	GObjectClass parent_class;
	void (*qset) (OhmStructure* self, GQuark field, GValue* value);
};
struct _OhmPattern {
	OhmStructure parent_instance;
	OhmPatternPrivate * priv;
};
struct _OhmPatternClass {
	OhmStructureClass parent_class;
};
/* , matches*/
struct _OhmPatternMatch {
	GObject parent_instance;
	OhmPatternMatchPrivate * priv;
};
struct _OhmPatternMatchClass {
	GObjectClass parent_class;
};
struct _OhmFact {
	OhmStructure parent_instance;
	OhmFactPrivate * priv;
};
struct _OhmFactClass {
	OhmStructureClass parent_class;
};
struct _OhmFactStore {
	GObject parent_instance;
	OhmFactStorePrivate * priv;
};
struct _OhmFactStoreClass {
	GObjectClass parent_class;
};
struct _OhmFactStoreChangeSet {
	GObject parent_instance;
	OhmFactStoreChangeSetPrivate * priv;
};
struct _OhmFactStoreChangeSetClass {
	GObjectClass parent_class;
};
struct _OhmFactStoreView {
	GObject parent_instance;
	OhmFactStoreViewPrivate * priv;
	OhmFactStoreChangeSet* change_set;
};
struct _OhmFactStoreViewClass {
	GObjectClass parent_class;
};
typedef enum  {
	OHM_FACT_STORE_EVENT_ADDED,
	OHM_FACT_STORE_EVENT_REMOVED,
	OHM_FACT_STORE_EVENT_UPDATED
} OhmFactStoreEvent;

/*FIXME*/
struct _OhmRule {
	GObject parent_instance;
	OhmRulePrivate * priv;
};
struct _OhmRuleClass {
	GObjectClass parent_class;
};

OhmStructure* ohm_structure_new (const char* name);
void ohm_structure_qset (OhmStructure* self, GQuark field, GValue* value);
GValue* ohm_structure_qget (OhmStructure* self, GQuark field);
void ohm_structure_set (OhmStructure* self, const char* field_name, GValue* value);
GValue* ohm_structure_get (OhmStructure* self, const char* field_name);
char* ohm_structure_to_string (OhmStructure* self);
void ohm_structure_value_to_string (GValue* src, GValue* dest);
GQuark ohm_structure_get_qname (OhmStructure* self);
char* ohm_structure_get_name (OhmStructure* self);
void ohm_structure_set_name (OhmStructure* self, const char* value);
GType ohm_structure_get_type (void);
OhmPattern* ohm_pattern_new (const char* name);
OhmPatternMatch* ohm_pattern_match (OhmPattern* self, OhmFact* fact, OhmFactStoreEvent event);
OhmFactStoreView* ohm_pattern_get_view (OhmPattern* self);
void ohm_pattern_set_view (OhmPattern* self, OhmFactStoreView* value);
OhmPatternMatch* ohm_pattern_match_new (OhmFact* fact, OhmPattern* pattern, OhmFactStoreEvent event);
char* ohm_pattern_match_to_string (OhmPatternMatch* self);
OhmFact* ohm_pattern_match_get_fact (OhmPatternMatch* self);
OhmPattern* ohm_pattern_match_get_pattern (OhmPatternMatch* self);
OhmFactStoreEvent ohm_pattern_match_get_event (OhmPatternMatch* self);
GType ohm_pattern_match_get_type (void);
GType ohm_pattern_get_type (void);
OhmFact* ohm_fact_new (const char* name);
GValue* ohm_fact_get (OhmFact* self, const char* field_name);
void ohm_fact_set (OhmFact* self, const char* field_name, GValue* value);
OhmFactStore* ohm_fact_get_fact_store (OhmFact* self);
void ohm_fact_set_fact_store (OhmFact* self, OhmFactStore* value);
GType ohm_fact_get_type (void);
gboolean ohm_fact_store_insert (OhmFactStore* self, OhmFact* fact);
void ohm_fact_store_remove (OhmFactStore* self, OhmFact* fact);
void ohm_fact_store_update (OhmFactStore* self, OhmFact* fact, GQuark field, GValue* value);
GSList* ohm_fact_store_get_facts_by_quark (OhmFactStore* self, GQuark qname);
GSList* ohm_fact_store_get_facts_by_name (OhmFactStore* self, const char* name);
GSList* ohm_fact_store_get_facts_by_pattern (OhmFactStore* self, OhmPattern* pattern);
OhmFactStore* ohm_fact_store_new (void);
char* ohm_fact_store_to_string (OhmFactStore* self);
void ohm_fact_store_my_debug (OhmFactStore* self);
OhmFactStore* ohm_fact_store_get_fact_store (void);
OhmFactStoreView* ohm_fact_store_new_view (OhmFactStore* self, GObject* listener);
void ohm_fact_store_change_set_add_match (OhmFactStoreChangeSet* self, OhmPatternMatch* match);
void ohm_fact_store_change_set_reset (OhmFactStoreChangeSet* self);
OhmFactStoreChangeSet* ohm_fact_store_change_set_new (void);
GSList* ohm_fact_store_change_set_get_matches (OhmFactStoreChangeSet* self);
GType ohm_fact_store_change_set_get_type (void);
GObject* ohm_fact_store_view_get_listener (OhmFactStoreView* self);
OhmFactStore* ohm_fact_store_view_get_fact_store (OhmFactStoreView* self);
GSList* ohm_fact_store_view_get_interested (OhmFactStoreView* self);
void ohm_fact_store_view_set_interested (OhmFactStoreView* self, GSList* value);
GType ohm_fact_store_view_get_type (void);
GType ohm_fact_store_event_get_type (void);
GType ohm_fact_store_get_type (void);
OhmRule* ohm_rule_new (OhmFactStore* fact_store);
void ohm_rule_fire (OhmRule* self);
OhmFactStore* ohm_rule_get_fact_store (OhmRule* self);
void ohm_rule_set_fact_store (OhmRule* self, OhmFactStore* value);
GSList* ohm_rule_get_interested (OhmRule* self);
void ohm_rule_set_interested (OhmRule* self, GSList* value);
GSList* ohm_rule_get_provide (OhmRule* self);
void ohm_rule_set_provide (OhmRule* self, GSList* value);
GType ohm_rule_get_type (void);
GValue ohm_value_from_string (const char* str);
GValue ohm_value_from_int (gint val);
GValue ohm_value_from_structure (OhmStructure* val);
OhmStructure* ohm_value_get_structure (GValue* value);


G_END_DECLS

#endif
