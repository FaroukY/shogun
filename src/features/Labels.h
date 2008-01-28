/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 1999-2008 Soeren Sonnenburg
 * Written (W) 1999-2008 Gunnar Raetsch
 * Copyright (C) 1999-2008 Fraunhofer Institute FIRST and Max-Planck-Society
 */

#ifndef _LABELS__H__
#define _LABELS__H__

#include "lib/common.h"
#include "lib/io.h"
#include "base/SGObject.h"

/** class Labels */
class CLabels : public CSGObject
{
	public:
		/** default constructor */
		CLabels();

		/** constructor
		 *
		 * @param num_labels number of labels
		 */
		CLabels(INT num_labels);

		/** constructor
		 *
		 * @param fname filename to load labels from
		 */
		CLabels(CHAR* fname);
		~CLabels();

		/** load labels from file
		 *
		 * @param fname filename to load from
		 * @return if loading was successful
		 */
		bool load(CHAR* fname);

		/** save labels to file
		 *
		 * @param fname filename to save to
		 * @return if saving was successful
		 */
		bool save(CHAR* fname);

		/** set label
		 *
		 * @param idx index of label to set
		 * @param label value of label
		 * @return if setting was successful
		 */
		inline bool set_label(INT idx, DREAL label)
		{ 
			if (labels && idx<num_labels)
			{
				labels[idx]=label;
				return true;
			}
			else 
				return false;
		}

		/** set INT label
		 *
		 * @param idx index of label to set
		 * @param label INT value of label
		 * @return if setting was successful
		 */
		inline bool set_int_label(INT idx, INT label)
		{ 
			if (labels && idx<num_labels)
			{
				labels[idx]= (DREAL) label;
				return true;
			}
			else 
				return false;
		}

		/** get label
		 *
		 * @param idx index of label to get
		 * @return value of label
		 */
		inline DREAL get_label(INT idx)
		{
			if (labels && idx<num_labels)
				return labels[idx];
			else
				return -1;
		}

		/** get INT label
		 *
		 * @param idx index of label to get
		 * @return INT value of label
		 */
		inline INT get_int_label(INT idx)
		{
			if (labels && idx<num_labels)
			{
				ASSERT(labels[idx]== ((DREAL) ((INT) labels[idx])));
				return ((INT) labels[idx]);
			}
			else
				return -1;
		}

		/** is two-class labeling
		 *
		 * @return if this is two-class labeling
		 */
		bool is_two_class_labeling();

		/** return number of classes (for multiclass)
		 * labels have to be zero based 0,1,...C missing
		 * labels are illegal
		 *
		 * @return number of classes
		 */
		INT get_num_classes();

		/** get labels
		 * caller has to clean up
		 *
		 * @param len number of labels
		 * @return the labels
		 */
		DREAL* get_labels(INT &len);
		
		/** get labels (swig compatible)
		 *
		 * @param labels where labels will be stored in
		 * @param len where number of labels will be stored in
		 */
		void get_labels(DREAL** labels, INT* len);

		/** set labels
		 *
		 * @param labels labels to set
		 * @param len number of labels
		 */
		void set_labels(DREAL* labels, INT len);

		/** get INT label vector
		 * caller has to clean up
		 *
		 * @param len number of labels to get
		 * @return INT labels
		 */
		INT* get_int_labels(INT &len);

		/** set INT labels
		 * caller has to clean up
		 *
		 * @param labels INT labels
		 * @param len number of INT labels
		 */
		void set_int_labels(INT *labels, INT len) ;

		/** get number of labels
		 *
		 * @return number of labels
		 */
		inline INT get_num_labels() { return num_labels; }
	protected:
		/** number of labels */
		INT num_labels;
		/** the labels */
		DREAL* labels;
};
#endif
