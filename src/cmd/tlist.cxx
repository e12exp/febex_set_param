
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
#include <type_traits>
#include <cctype> //tolower
#include <string.h>
#include "assert.h"

int get_val_range(int sfp_first, int sfp_last, int module_first, int module_last[4],
		  int channel_first, int channel_last,
		  conf_value_def_t *vardef,
		  int64_t& val_min, int64_t& val_max
		  )
{
  file_data_t *file = g_file_data[g_active_file];
  
  int global=vardef->global;
  if (vardef->type!=conf_type_mask)
    {
      val_max=std::numeric_limits<std::remove_reference<decltype(val_max)>::type >::min();
      val_min=std::numeric_limits<std::remove_reference<decltype(val_min)>::type >::max();
    }
  else
    {
      val_max=0;
      val_min=~(std::remove_reference<decltype(val_min)>::type)(0); // all ones
    }
  int count=0;
  for(int sfp = sfp_first; sfp <= sfp_last; sfp++)
    for(int mod = module_first; mod <= module_last[sfp]; mod++)
      for(int c = (global?-1:channel_first); c <=(global?-1:channel_last); c++)
	{
	  int32_t *conf_val=module_data_get(file, sfp, mod, c, vardef->name, NULL, NULL, NULL);
	  assert(conf_val);
		      
	  int64_t val=vardef->vsigned?(int64_t)*conf_val:(int64_t)(uint64_t)*conf_val;
	  count++;
	  if(vardef->type != conf_type_mask)
	    {
	      val_max=std::max(val_max, val);
	      val_min=std::min(val_min, val);
	    }
	  else
	    {
	      val_max|=val;
	      val_min&=val;
	    }
	  
	  //printf("val, val_min, mal_max: %llx %llx %llx\n", val, val_min, val_max);
	}
  // end for loop

  //printf("val_min, mal_max: %llx %llx\n", val_min, val_max);

  //printf("count %d:\n", count);

  return 0;
}


IMPL(tlist)
{
  ARGS_INIT
  ARG_STR(channelspec)
        
  file_data_t *file = g_file_data[g_active_file];

  conf_value_def_t *vardef=NULL;
  
  int64_t val_min, val_max; // maximum and minimum value encountered, not allowed range!

  int sfp_first, sfp_last, module_first, module_last[4], channel_first, channel_last, sfp, mod;

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
	      memcpy(mod_wildcard, buf, i);
	      mod_wildcard[i]=0;
	    }
	  else if (num_dot==3)
	    {	
	      memcpy(ch_wildcard, buf, i);
	      ch_wildcard[i]=0;      
	    }
	}
    assert(num_dot==3);
  }
  
  char * name;
  if(!_set_get_interpret_path(buf, &sfp_first, &sfp_last, &module_first, module_last,
			      &channel_first, &channel_last, &name))
    return 0;  // Note: module_last is an array -- no & needed. 

  firmware_def_t *fw=0;
  {
    for (sfp=sfp_first; sfp<=sfp_last; sfp++)
      for(mod = module_first; mod <= module_last[sfp]; mod++)
	if (!fw)
	  fw = file->module_data[sfp][mod].firmware;

  
    if (!fw)
      {
	fprintf(stderr, "tlist error: no modules found in range!\n");
	fprintf(stderr, "  sfp range: %d - %d\n  module_first: %d\n", sfp_first, sfp_last, module_first);
	for (sfp=sfp_first; sfp<=sfp_last; sfp++)
	  fprintf(stderr, "  module_last[%d]=%d\n", sfp, module_last[sfp]);
	fprintf(stderr, "  channel range: %d - %d\n:", channel_first, channel_last);
	return 0;
      }
  }

  
  int addr_len=strlen(ch_wildcard)+1+strlen("trigger_validation_src");
  char addr_format[150];
  snprintf(addr_format, 150, " %%-%ds %%s", addr_len+1);

  
  printf("\n   ===== Module trigger bus (rows) <-> Backplane (cols) trigger matrix =====\n\n");
  const auto modmatrix_header=
    {
      "    === Backplane Lines (2 bit/col!) ===\n",
      "  /- B6: Busy (typ. OUT)\n",
      "  |  /- B5: General Purpose (IN/OUT)\n",
      "  |  |  /- B4: Readout Request (typ. OUT)\n",
      "  |  |  |  /- B3: General Purpose (IN/OUT)\n",
      "  |  |  |  |  /- B2: Readout Trigger T3 (typ. IN)\n",
      "  |  |  |  |  |  /- B1: Readout Trigger T2 (unused), G.P. (IN/OUT)\n",
      "  |  |  |  |  |  |  /- B0: Readout Trigger T1 (typ. IN)\n"//,
      //      " 6| 5| 4| 3| 2| 1| 0| \n"
    };
  for (auto &name : modmatrix_header)
    printf(addr_format, "", name);
  printf(addr_format, "== module triger lines ==",       " 6| 5| 4| 3| 2| 1| 0| \n");
  const auto modmatrix=\
    {
      "iomatrix_gpt1", "iomatrix_gpt2", "iomatrix_gpt3", "iomatrix_gpt4",
      "iomatrix_t1",   "iomatrix_t3",   "iomatrix_rorq", "iomatrix_busy", "veto_dst"
    };

  
  const char ioval[]=
    {
      '.', // 00: NC
      'I', // 01: IN
      'O', // 10: OUT
      'X'  // 11: IN (nonstandard)
    };
  
  for (auto &name : modmatrix)
    {
      auto res=module_data_get(file, 0, 0, -1, const_cast<char*>(name), 0, 0, &vardef);
      assert(res);
      assert(vardef);
      get_val_range(sfp_first, sfp_last, module_first, module_last,	
		    channel_first, channel_last,
		    vardef,
		    val_min,val_max);
      char buf[150];
      snprintf(buf, 150, "%s.%s %s", mod_wildcard, name, strcmp(name,"veto_dst")?"":"(\?\?)");
      printf(addr_format, buf, "");
      for (int i=6; i>=0; i--)
	{
	  int vmin=0x3 & (val_min>>(2*i));
	  int vmax=0x3 & (val_max>>(2*i));
	  
	  if (vmin==vmax)
	    printf("  %1c", ioval[vmax]);
	  else
	    printf("  %1c", (char)tolower(ioval[vmax]));
	}
      printf ("   0b");
      //      printf("\nbm=%x\n", vardef->bitmask);
      print_binary_val(val_min, val_max, 0x3fff);
      printf(" aka 0x%04llx %s", val_max, (val_min==val_max)?"":"(some)");
      printf("\n");
    }

  printf("\n"
	 "       '.': NC (0b00)    'I': input (0b01)     'O': output (0b10)\n"
	 "       'X': (nonstd) input (0b11).      Lowercase (i, o, x): mixed.\n");

  printf("\n   ===== Channel trigger (rows) <-> Module (cols) trigger matrix =====\n\n");
  const auto chmatrix_header=
    {
      "   ==== same channel discriminators, src only ===\n",
      "  /- 9: (0x200) proton discriminator\n",
      "  |  /- 8: (0x100) gamma discriminator\n",
      "  |  |  /- 7: (0x80) timing discriminator \n",
      "  |  |  |   ==== module trigger bus ====\n",
      "  |  |  |  /- 6: (0x40) Readout Request   \n",
      "  |  |  |  |  /- 5: (0x20) Readout T3 (src only)\n",
      "  |  |  |  |  |  /- 4: (0x10) Readout T1 (src only)\n",
      "  |  |  |  |  |  |  /- 3: (0x8) GPT4\n",
      "  |  |  |  |  |  |  |  /- 2: (0x4) GPT3\n",
      "  |  |  |  |  |  |  |  |  /- 1: (0x2) GPT2 \n",
      "  |  |  |  |  |  |  |  |  |  /- 0: (0x1) GPT1\n"//,
      //      " 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| \n"
    };

  for (auto &name : chmatrix_header)
    printf(addr_format, "", name);


  char chansyms[]={'.', '?', 'X'};
  auto chmatrix={
    "trigger_timing_src",
    "trigger_validation_src",
    "trigger_timing_dst",
    "trigger_gamma_dst",
    "trigger_proton_dst"
  };
  
  printf(addr_format, "== channel evt trig inputs ==", " 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| \n");
  int second_header=0;
  for (auto &name : chmatrix)
    {
      auto res=module_data_get(file, 0, 0, 0, const_cast<char*>(name), 0, 0, &vardef);
      assert(res);
      assert(vardef);
      get_val_range(sfp_first, sfp_last, module_first, module_last,	
		    channel_first, channel_last,
		    vardef,
		    val_min,val_max);

      char buf[150];
      bool is_dst=!strcmp(const_cast<char*>(name)+strlen(name)-3, "dst");
      if (is_dst && ! second_header++)
	printf(addr_format, "== channel discr outputs ==", " 9| 8| 7| 6| 5| 4| 3| 2| 1| 0| \n");
      
      snprintf(buf, 150, "%s.%s ", ch_wildcard, name);
      printf(addr_format, buf, "");
      for (int i=9; i>=0; i--)
	{
	  int vmin=0x1 & (val_min>>(i));
	  int vmax=0x1 & (val_max>>(i));
	  char sym;
	  if (is_dst && (i>=7 || i==4 || i==5))
	      sym=' ';
	  else
	    sym= chansyms[vmin+vmax];
	  printf("  %1c", sym);
	}
      
      //      printf("\nbm=%x\n", vardef->bitmask);
      printf("   0b");
      print_binary_val(val_min, val_max, 0x3ff);
      printf(" aka 0x%03llx %s", val_max, (val_min==val_max)?"":"(some)");
      printf("\n");
    }
  printf("\n"
	 "       ' ': always NC    '.': NC    'X': connected    '?' some connected\n"
	 "       Note: Some other settings also affect triggers, use slist to view.\n");
	 
  
  return 1;
}


IMPL_HELP(tlist)
{
  printf("Trigger listing of one or more modules.\n"
	 "  channelspec: a <sfp>.<module>.<channel> specification which may contain wildcards)\n"
	 "               (as in the \"get\" command).\n"
	 "Will print the current FPGA trigger matrix setup.\n"
	 "tlist uses lowercase letters (o,i,x) to indicate settings which are different between\n"
	 "modules.\n"
	 );
  return 1;
}

