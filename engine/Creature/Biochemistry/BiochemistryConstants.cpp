#include "BiochemistryConstants.h"





TableSpec ourReceptorLocusNumbers[] =
{
	TableSpec("Receptor Locus Numbers"),
	TableSpec("\'Organ\'", "Tissue", "Locus ID", "Name", "Description"),

	TableSpec("0 Brain", "Lobe tissue ID defined in genome", "0-3", "Neuron 0 states 0 to 3.", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "4-7", "Neuron 1 states 0 to 3.", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "...", "...", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "252-255", "Neuron 63 states 0 to 3.", "&nbsp;"),
	TableSpec(),

	TableSpec("1 Creature", "0 Somatic", "0", "Age 0", "If on and currently BABY, become CHILD."),
	TableSpec("&nbsp;", "&nbsp;", "1", "Age 1", "If on and currently CHILD, become ADOLESCENT."),
	TableSpec("&nbsp;", "&nbsp;", "2", "Age 2", "If on and currently ADOLESCENT, become YOUTH."),
	TableSpec("&nbsp;", "&nbsp;", "3", "Age 3", "If on and currently YOUTH, become ADULT."),
	TableSpec("&nbsp;", "&nbsp;", "4", "Age 4", "If on and currently ADULT, become OLD."),
	TableSpec("&nbsp;", "&nbsp;", "5", "Age 5", "If on and currently OLD, become SENILE."),
	TableSpec("&nbsp;", "&nbsp;", "6", "Age 6", "If on DIE IMMEDIATELY of old age"),
	TableSpec(),

	TableSpec("&nbsp;", "1 Circulatory", "0-31", "Floating", "&nbsp;"),
	TableSpec(),

	TableSpec("&nbsp;", "2 Reproductive", "0", "Ovulate", "If low, remove any egg/sperm from .Gamete; if high, add one"),
	TableSpec("&nbsp;", "&nbsp;", "1", "Receptive", "If >0, female is receptive to incoming sperm & will conceive"),
	TableSpec("&nbsp;", "&nbsp;", "2", "Chanceofmutation", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "3", "Degreeofmutation", "&nbsp;"),
	TableSpec(),

	TableSpec("&nbsp;", "3 Immune", "0", "Die", "If on, creature dies (ill health, poison, starvation...)"),
	TableSpec(),

	TableSpec("&nbsp;", "4 Sensorimotor", "0", "Involuntary 0", "Trigger flinching."),
	TableSpec("&nbsp;", "&nbsp;", "1", "Involuntary 1", "Lay egg."),
	TableSpec("&nbsp;", "&nbsp;", "2", "Involuntary 2", "Sneeze."),
	TableSpec("&nbsp;", "&nbsp;", "3", "Involuntary 3", "Cough."),
	TableSpec("&nbsp;", "&nbsp;", "4", "Involuntary 4", "Shiver."),
	TableSpec("&nbsp;", "&nbsp;", "5", "Involuntary 5", "Sleep"),
	TableSpec("&nbsp;", "&nbsp;", "6", "Involuntary 6", "Fainting"),
	TableSpec("&nbsp;", "&nbsp;", "7", "Involuntary 7", "Drowning"),
	TableSpec(),

	TableSpec("&nbsp;", "&nbsp;", "8", "Gait 0", "Trigger various walking gaits (0=default)."),
	TableSpec("&nbsp;", "&nbsp;", "9", "Gait 1", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "10", "Gait 2", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "11", "Gait 3", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "12", "Gait 4", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "13", "Gait 5", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "14", "Gait 6", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "15", "Gait 7", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "16", "Gait 8", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "17", "Gait 9", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "18", "Gait 10", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "19", "Gait 11", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "20", "Gait 12", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "21", "Gait 13", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "22", "Gait 14", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "23", "Gait 15", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "24", "Gait 16", "&nbsp;"),
	TableSpec(),

	// Drives
	TableSpec("&nbsp;", "5 Drives", "0", "Drive 0", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "1", "Drive 1", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "2", "Drive 2", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "3", "Drive 3", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "4", "Drive 4", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "5", "Drive 5", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "6", "Drive 6", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "7", "Drive 7", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "8", "Drive 8", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "9", "Drive 9", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "10", "Drive 10", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "11", "Drive 11", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "12", "Drive 12", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "13", "Drive 13", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "14", "Drive 14", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "15", "Drive 15", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "16", "Drive 16", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "17", "Drive 17", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "18", "Drive 18", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "19", "Drive 19", "&nbsp;"),
	TableSpec(),
};


TableSpec ourEmitterLocusNumbers[] =
{
	TableSpec("Emitter Locus Numbers"),
	TableSpec("\'Organ\'", "Tissue", "Locus ID", "Name", "Description"),

	TableSpec("0 Brain", "Lobe tissue ID defined in genome", "0-3", "Neuron 0 states 0 to 3.", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "4-7", "Neuron 1 states 0 to 3.", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "...", "...", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "252-255", "Neuron 63 states 0 to 3.", "&nbsp;"),
	TableSpec(),

	TableSpec("1 Creature", "&nbsp;", "&nbsp;", "&nbsp;", "&nbsp;"),
	TableSpec("&nbsp;", "0 Somatic", "&nbsp;", "&nbsp;", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "0", "Muscles", "How much energy has been expended on movement this tick."),
	TableSpec(),

	TableSpec("&nbsp;", "1 Circulatory", "0-31", "Floating", "&nbsp;"),
	TableSpec(),

	TableSpec("&nbsp;", "2 Reproductive", "0", "Fertile", "If male has a sperm or female has an egg available"),
	TableSpec("&nbsp;", "&nbsp;", "1", "Pregnant", "If female has both egg and sperm & so is pregnant"),
	TableSpec("&nbsp;", "&nbsp;", "2", "Ovulate", "If low, remove any egg/sperm from .Gamete; if high, add one"),
	TableSpec("&nbsp;", "&nbsp;", "3", "Receptive", "If >0, female is receptive to incoming sperm & will conceive"),
	TableSpec("&nbsp;", "&nbsp;", "4", "Chance of Mutation", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "5", "Degree of Mutation", "&nbsp;"),
	TableSpec(),

	TableSpec("&nbsp;", "3 Immune", "0", "Dead", ">0 if creature is dead (allows post-mortem chemistry)"),
	TableSpec(),

	TableSpec("&nbsp;", "4 Sensorimotor", "0", "Constant", "Always 1.0 (for regular emitters)."),
	TableSpec("&nbsp;", "&nbsp;", "1", "Asleep", "1.0 if asleep, 0.0 otherwise."),
	TableSpec("&nbsp;", "&nbsp;", "2", "Coldness (DEPRECATED)", ""),
	TableSpec("&nbsp;", "&nbsp;", "3", "Hotness (DEPRECATED)", ""),
	TableSpec("&nbsp;", "&nbsp;", "4", "Lightlevel (DEPRECATED)", ""),
	TableSpec("&nbsp;", "&nbsp;", "5", "Crowdedness", "How many and how close others of your kind are"),
	TableSpec("&nbsp;", "&nbsp;", "6", "Radiation (DEPRECATED)", ""),
	TableSpec("&nbsp;", "&nbsp;", "7", "Timeofday (DEPRECATED)", ""),
	TableSpec("&nbsp;", "&nbsp;", "8", "Season (DEPRECATED)", ""),
	TableSpec("&nbsp;", "&nbsp;", "9", "Air Quality", "How breathable is the air (0.0 for air, 1.0 for water)."),
	TableSpec("&nbsp;", "&nbsp;", "10", "Upslope", "How steep is the slope I'm facing"),
	TableSpec("&nbsp;", "&nbsp;", "11", "Downslope", "How steep is the slope I'm facing"),
	TableSpec("&nbsp;", "&nbsp;", "12", "Headwind (DEPRECATED)", ""),
	TableSpec("&nbsp;", "&nbsp;", "13", "Tailwind (DEPRECATED)", ""),
	TableSpec(),

	// Drives
	TableSpec("&nbsp;", "5 Drives", "0", "Drive 0", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "1", "Drive 1", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "2", "Drive 2", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "3", "Drive 3", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "4", "Drive 4", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "5", "Drive 5", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "6", "Drive 6", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "7", "Drive 7", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "8", "Drive 8", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "9", "Drive 9", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "10", "Drive 10", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "11", "Drive 11", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "12", "Drive 12", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "13", "Drive 13", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "14", "Drive 14", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "15", "Drive 15", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "16", "Drive 16", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "17", "Drive 17", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "18", "Drive 18", "&nbsp;"),
	TableSpec("&nbsp;", "&nbsp;", "19", "Drive 19", "&nbsp;"),
	TableSpec(),
};

int sizeOurReceptorLocusNumbers = sizeof(ourReceptorLocusNumbers) / sizeof(TableSpec);
int dummyReceptorLocusNumbers = AutoDocumentationTable::RegisterTable(ourReceptorLocusNumbers, sizeof(ourReceptorLocusNumbers));

int sizeOurEmitterLocusNumbers = sizeof(ourEmitterLocusNumbers) / sizeof(TableSpec);
int dummyEmitterLocusNumbers = AutoDocumentationTable::RegisterTable(ourEmitterLocusNumbers, sizeof(ourEmitterLocusNumbers));
