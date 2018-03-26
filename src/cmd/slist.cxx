
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "../command.h"
#include "../data.h"
#include "../consoleinterface.h"
#include "../paramdef.h"
#include "../binaryval.h"
#include "slist.h"
#include "set.h" //_set_get_interpret_path
#include <limits>
#include <algorithm>

#include <string.h>
#include "assert.h"

IMPL(slist)
{
  ARGS_INIT
  ARG_STR(channelspec)
        
  file_data_t *file = g_file_data[g_active_file];

  int c;
  conf_value_def_t *vardef;
  
  int64_t val_min, val_max, val; // maximum and minimum value encountered, not allowed range!

  int sfp_first, sfp_last, module_first, module_last[4], channel_first, channel_last, sfp, mod;
  char *name;

  int num_dot=0;
  for (int i=0; channelspec[i]; i++)
    num_dot+=channelspec[i]=='.';
    
  char buf[100];
  snprintf(buf, 100, "%s%s%s.foo", channelspec, num_dot<2?".*":"", num_dot<1?".*":"");
  
  //  printf("buf=%s\n", buf);

  char ch_wildcard[100];
  char mod_wildcard[100];

  {
    assert(strlen(buf)+1<100);
    int num_dot=0;
    for (int i=0; buf[i]!=0; i++)
      if (buf[i]=='.')
	{
	  num_dot++;
	  if (num_dot==2)
	    {
	      memcpy(ch_wildcard, buf, i);
	      ch_wildcard[i]=0;
	    }
	  else if (num_dot==3)
	    {	
	      memcpy(mod_wildcard, buf, i);
	      mod_wildcard[i]=0;      
	    }
	}
    assert(num_dot==3);
  }

  if(!_set_get_interpret_path(buf, &sfp_first, &sfp_last, &module_first, module_last, &channel_first, &channel_last, &name))
    return 0; 

  firmware_def_t *fw=0;
  {
    for (sfp=sfp_first; sfp<=sfp_last; sfp++)
      for(mod = module_first; mod <= module_last[sfp]; mod++)
	if (!fw)
	  fw = file->module_data[sfp][mod].firmware;

  
    if (!fw)
      {
	fprintf(stderr, "slist error: no modules found in range!\n");
	fprintf(stderr, "  sfp range: %d - %d\n  module_first: %d\n", sfp_first, sfp_last, module_first);
	for (sfp=sfp_first; sfp<=sfp_last; sfp++)
	  fprintf(stderr, "  module_last[%d]=%d\n", sfp, module_last[sfp]);
	fprintf(stderr, "  channel range: %d - %d\n:", channel_first, channel_last);
	return 0;
      }
  }

  
  int addr_len=strlen(ch_wildcard)+1+strlen("proton_trigger_walk_correction");
  char addr_format[150];
  snprintf(addr_format, 150, "    %%-%ds (0x%%06x)  ", addr_len+1);
  for (int section=0; section<NUMCATEGORIES; section++)
    {
      if (!fw->categories[section])
	continue;
      char sep[200];
      char last=fw->categories[section]->name[strlen(fw->categories[section]->name)-1];
      memset(sep, last, 200);
      int sepstop=std::max(75-(int)strlen(fw->categories[section]->name), 0);
      sepstop=std::min(sepstop, 200);
      sep[sepstop]=0;
      printf("%s%s\n", fw->categories[section]->name, sep);   
      for (int subsection=0; subsection<PARAMSPERCATEGORY; subsection++)
	if ((vardef=fw->categories[section]->params[subsection]))
	  {
	    int global=vardef->global;
	    char buf[150];
	    snprintf(buf, 150, "%s.%s", global?mod_wildcard:ch_wildcard, vardef->name);
	    printf(addr_format, buf, vardef->addr);
	    if (vardef->type!=conf_type_mask)
	      {
		val_max=std::numeric_limits<decltype(val_max)>::min();
		val_min=std::numeric_limits<decltype(val_min)>::max();
	      }
	    else
	      {
		val_max=0;
		val_min=~decltype(val_min)(0); // all ones
	      }
	    for(sfp = sfp_first; sfp <= sfp_last; sfp++)
	      for(mod = module_first; mod <= module_last[sfp]; mod++)
		for(c = (global?-1:channel_first); c <=(global?-1:channel_last); c++)
		  {
		    int32_t *conf_val=module_data_get(file, sfp, mod, c, vardef->name, NULL, NULL, NULL);
		    assert(conf_val);
		      
		    val=vardef->vsigned?(int64_t)*conf_val:(int64_t)(uint64_t)*conf_val;
		    if(vardef->type != conf_type_mask)
		      {
			val_max=std::max(val_max, val);
			val_min=std::min(val_min, val);
		      }
		    else
		      {
			if (0 && !strcmp(vardef->name, "trigger_timing_dst"))
			  {
			    printf("val: %llx\n", val);
			    if (val_max!=(val_max|val))
			      printf("max: %llx -> %llx\n", val_max, val_max|val);
			    if (val_min!=(val_min|val))
			      printf("min: %llx -> %llx\n", val_min, val_min|val);
			  }
			val_max|=val;
			val_min&=val;
		      }
		  }
	    // end for loop

	      
	    printf("%5s ", (val_min!=val_max)?"MIXED":"");

	    const char* fmt=(vardef->unit)?"%-6lld":"%lld";
	    
	    if(vardef->type != conf_type_mask)
	      {
		if (val_min!=val_max)
		  {
		    printf(fmt, val_min);
		    printf(" - ");
		    printf(fmt, val_max);
		  }
		else
		  {
		    printf(fmt, val_max);
		  }
	      }
	    else // bitmask
	      {
		uint32_t bitmask=vardef->bitmask >> vardef->lowbit;
		if (val_min!=val_max)
		  {
		    printf("0x%04llx ^ 0x%04llx  ", (val_min>>vardef->lowbit)&bitmask, (val_max>>vardef->lowbit)&val_max&bitmask);
		  }
		else
		  {
		    printf("0x%04llx ", val_min);
		  }
		printf("aka 0b");
		print_binary_val(val_min, val_max, bitmask);
		printf(" ");
	      }

	    
	    if(vardef->unit != NULL)
	      {
		printf("  (x %s) ", vardef->unit);
	      }
	    
	    printf("\n");

	    //	    printf("\n");
	  } //subsection/var loop
    } // loop categories
  return 1;
}


IMPL_HELP(slist)
{
  printf("Sorted, shortened, smart list of configuration parameters of one or more modules.\n"
	 "  channelspec: a <sfp>.<module>.<channel> specification which may contain wildcards)\n"
	 "               (as in the \"get\" command).\n"
	 "Will show the config parameters sorted by categories and aggregated over all channels specified.\n"
	 "slist only shows range for parameters which are set different in individual channels, use \"get\"\n"
	 " to display the the full range of values.\n"
	 );
  return 1;
}

