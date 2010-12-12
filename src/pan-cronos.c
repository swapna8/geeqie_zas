#include "main.h"
#include "pan-types.h"

#include <glib/gprintf.h>
#include <math.h>


#define PAN_CAL_POPUP_COLOR 220, 220, 220
#define PAN_CAL_POPUP_ALPHA 255
#define PAN_CAL_POPUP_BORDER 1
#define PAN_CAL_POPUP_BORDER_COLOR 0, 0, 0
#define PAN_CAL_POPUP_TEXT_COLOR 0, 0, 0

#define PAN_CAL_DAY_WIDTH 100
#define PAN_CAL_DAY_HEIGHT 80

#define PAN_CAL_DAY_COLOR 255, 255, 255
#define PAN_CAL_DAY_ALPHA 220
#define PAN_CAL_DAY_BORDER 2
#define PAN_CAL_DAY_BORDER_COLOR 0, 0, 0
#define PAN_CAL_DAY_TEXT_COLOR 0, 0, 0

#define PAN_CAL_MONTH_COLOR 255, 255, 255
#define PAN_CAL_MONTH_ALPHA 200
#define PAN_CAL_MONTH_BORDER 4
#define PAN_CAL_MONTH_BORDER_COLOR 0, 0, 0
#define PAN_CAL_MONTH_TEXT_COLOR 0, 0, 0

#define PAN_CAL_DOT_SIZE 3
#define PAN_CAL_DOT_GAP 2
#define PAN_CAL_DOT_COLOR 128, 128, 128
#define PAN_CAL_DOT_ALPHA 128

gint current_depth_date;
gint depth_date;

gboolean pan_cronos_more_to_show(PanWindow *pw,time_t dt);
/*
 *-----------------------------------------------------------------------------
 * cronos
 *-----------------------------------------------------------------------------
 */

void pan_cronos_slideshow_stop(PanWindow *pw)
{
  if (!pw) return;

  slideshow_free(pw->slide_show);
}

void pan_cronos_update(PanWindow *pw,int advance,PanItem *pi)
{
   /** We advance or go backwards through cronos */
   if (advance==1)
   {
   if (pw->depth == PAN_DATE_LENGTH_YEAR && advance==1) pw->depth = PAN_DATE_LENGTH_MONTH;
   else if (pw->depth == PAN_DATE_LENGTH_MONTH && advance==1) pw->depth = PAN_DATE_LENGTH_DAY;
     
   
   else if (pw->depth == PAN_DATE_LENGTH_DAY && advance==1) pw->depth = PAN_DATE_LENGTH_EXACT;
  

   else if (pw->depth == PAN_DATE_LENGTH_EXACT && advance==1) pw->depth = 99; /** It means:start slideshow */
   }

   else
   {
 if (pw->depth == 99 && advance==-1) pw->depth = PAN_DATE_LENGTH_EXACT;
 else if (pw->depth == PAN_DATE_LENGTH_EXACT && advance==-1) pw->depth = PAN_DATE_LENGTH_DAY;
   else if (pw->depth == PAN_DATE_LENGTH_DAY && advance==-1) pw->depth = PAN_DATE_LENGTH_MONTH;
   else if (pw->depth == PAN_DATE_LENGTH_MONTH && advance==-1) pw->depth = PAN_DATE_LENGTH_YEAR;


   return;
   }
  
   switch(pw->depth)
   {

	case PAN_DATE_LENGTH_EXACT:
	pw->depth = PAN_DATE_LENGTH_EXACT;
	pw->current_day = pan_date_value (pi->fd->date,PAN_DATE_LENGTH_DAY);
	break;

 	case PAN_DATE_LENGTH_DAY:
	pw->depth = PAN_DATE_LENGTH_DAY;
	pw->current_month = pan_date_value (pi->fd->date,PAN_DATE_LENGTH_MONTH);
	break;

        case PAN_DATE_LENGTH_MONTH:
   	pw->depth = PAN_DATE_LENGTH_MONTH;
	pw->current_year = pan_date_value (pi->fd->date,PAN_DATE_LENGTH_YEAR);
	break;


   
   }
}

GList *glist_split(PanWindow *pw,GList *list)
{
  FileData *fd;
  FileData *fd_prev=NULL;
  gboolean begin=FALSE;
  GList *res=NULL;
  GList *work=NULL;

 
  work = list;
DEBUG_1("List size before split %d",g_list_length(work));

  fd = work->data;
  depth_date = 0;
  current_depth_date = 1;
  /** Find first date that matchs */
  while(work && !pan_cronos_more_to_show(pw,fd->date))
  {  
    fd = work->data;
    work = work->next; 
  }
    /** Starts addiding days until it founds a different date */
  fd = work->data;
    while(work && pan_cronos_more_to_show(pw,fd->date))
  {
    if (fd!=fd_prev) res = g_list_append(res,fd);
    fd_prev = fd;
    fd = work->data;
    work = work->next;
   
  }

    DEBUG_1("List size %d",g_list_length(res));

  return res;

 
}
/** TODO: Add n thumbnails control */
gboolean pan_cronos_more_to_show(PanWindow *pw,time_t dt)
{
   gint current_year;
   gint current_month; 
   gint current_day;

   switch(pw->depth)
   {
      case PAN_DATE_LENGTH_YEAR:
      return (current_depth_date!=depth_date);
      break;

      case PAN_DATE_LENGTH_MONTH:
      current_year = pan_date_value(dt, PAN_DATE_LENGTH_YEAR);
      return (current_depth_date!=depth_date) && (pw->current_year == current_year);
      break;

      case PAN_DATE_LENGTH_DAY:
      current_year = pan_date_value(dt, PAN_DATE_LENGTH_YEAR);
      current_month = pan_date_value(dt,PAN_DATE_LENGTH_MONTH); 
      return (current_depth_date!=depth_date) && (pw->current_year == current_year) && (pw->current_month == current_month);
      break;

      case PAN_DATE_LENGTH_EXACT:
      current_year = pan_date_value(dt, PAN_DATE_LENGTH_YEAR);
      current_month = pan_date_value(dt,PAN_DATE_LENGTH_MONTH); 
      current_day = pan_date_value(dt,PAN_DATE_LENGTH_DAY);
      return (pw->current_day == current_day) && (pw->current_year == current_year) && (pw->current_month == current_month);
      break;
         
   }

}

void pan_cronos_calendar_compute(PanWindow *pw,FileData *dir_fd,gint *width,gint *height)
{
	GList *list;
	GList *work;
	gint x, y;
	time_t tc;
	gint count;
	gint day_max;
	gint day_width;
	gint day_height;
	gint grid;
	gint year = 0;
	gint month = 0;
	gint end_year = 0;
	gint end_month = 0;
	/** Get all files from directory dir_fd and turns them into a FileData object */
	list = pan_list_tree(dir_fd, SORT_NONE, TRUE, pw->ignore_symlinks);
	/** If cache is enabled and exif data; sort it BY NAME, and sync with new list */
	if (pw->cache_list && pw->exif_date_enable)
		{
		pw->cache_list = pan_cache_sort(pw->cache_list, SORT_NAME, TRUE);
		list = filelist_sort(list, SORT_NAME, TRUE);
		pan_cache_sync_date(pw, list);
		}
        /** Now sync it by time , to get earlier pics at first of the list */
	pw->cache_list = pan_cache_sort(pw->cache_list, SORT_TIME, TRUE);
	list = filelist_sort(list, SORT_TIME, TRUE);

	day_max = 0;
	count = 0;
	tc = 0;
	work = list;
	while (work)
		{
		FileData *fd;

		fd = work->data;
		work = work->next;
		/** Compares new considered file date with day 'tc'. If tc=fd->date ; then increments the total
		    amount of pics found for that day; and updates day_max if that day has more pics than
		    all the days considered since now.If not, it updates 'tc' with the new day, and resets counting */
		if (!pan_date_compare(fd->date, tc, PAN_DATE_LENGTH_DAY))
			{
			count = 0;
			tc = fd->date;
			}
		else
			{
			count++;
			if (day_max < count) day_max = count;
			}
		}

	DEBUG_1("biggest day contains %d images", day_max);
	/** Grid computing.Grid size is determinated by 'day_max', so it ensured that all days will fit it,because
	    they have less pics than 'day_max'.
            Day width and height are MAX between grid and their constants; in order to look pretty*/
	grid = (gint)(sqrt((gdouble)day_max) + 0.5) * (PAN_THUMB_SIZE + PAN_SHADOW_OFFSET * 2 + PAN_THUMB_GAP);
	day_width = MAX(PAN_CAL_DAY_WIDTH, grid);
	day_height = MAX(PAN_CAL_DAY_HEIGHT, grid);

	/** Gets first and last year and month of the pics */
	if (list)
		{
		FileData *fd = list->data;

		year = pan_date_value(fd->date, PAN_DATE_LENGTH_YEAR);
		month = pan_date_value(fd->date, PAN_DATE_LENGTH_MONTH);
		}

	work = g_list_last(list);
	if (work)
		{
		FileData *fd = work->data;
		end_year = year;
		end_month = month;
		}
	/** Update height and weigth with borders length; and sets x and y to their value,respectively*/
	*width = PAN_BOX_BORDER * 2;
	*height = PAN_BOX_BORDER * 2;

	x = PAN_BOX_BORDER;
	y = PAN_BOX_BORDER;

	work = (GList *)glist_split(pw,list);
	year = pw->current_year;
	month = pw->current_month;
	end_year = year;
        end_month = month;
	/** While we have¡n reached end_year; and if so we haven't reached end_motnh, do work */
	while (work && (year < end_year || (year == end_year && month <= end_month)))
		{
		PanItem *pi_month;
		PanItem *pi_text;
		PanItem *pi_image;
		gint day;
		gint days;
		gint col;
		gint row;
		time_t dt;
		gchar *buf;

		/* figure last second of this month */
		dt = pan_date_to_time((month == 12) ? year + 1 : year, (month == 12) ? 1 : month + 1, 1);
		dt -= 60 * 60 * 24;

		/* anything to show this month? */
		/*	if (!pan_date_compare(((FileData *)(work->data))->date, dt, PAN_DATE_LENGTH_MONTH))
			{
			month ++;
			if (month > 12)
				{
				year++;
				month = 1;
				}
			//continue;
			}*/
		/** Get day and week values */
		days = pan_date_value(dt, PAN_DATE_LENGTH_DAY);
		dt = pan_date_to_time(year, month, 1);
		col = pan_date_value(dt, PAN_DATE_LENGTH_WEEK);
		row = 1;

		x = PAN_BOX_BORDER;
		/** New pan item : month */
		pi_month = pan_item_box_new(pw, NULL, x, y, PAN_CAL_DAY_WIDTH * 7, PAN_CAL_DAY_HEIGHT / 4,
					    PAN_CAL_MONTH_BORDER,
					    PAN_CAL_MONTH_COLOR, PAN_CAL_MONTH_ALPHA,
					    PAN_CAL_MONTH_BORDER_COLOR, PAN_CAL_MONTH_ALPHA);
		buf = pan_date_value_string(dt, PAN_DATE_LENGTH_MONTH);

		/** Date text */
		pi_text = pan_item_text_new(pw, x, y, buf,
					    PAN_TEXT_ATTR_BOLD | PAN_TEXT_ATTR_HEADING,
					    PAN_TEXT_BORDER_SIZE,
					    PAN_CAL_MONTH_TEXT_COLOR, 255);
		g_free(buf);
		/** pi_text x and y position.To make it start at left upper corner of month box. */
		pi_text->x = pi_month->x + (pi_month->width - pi_text->width) / 2;
	
		pi_month->height = pi_text->y + pi_text->height - pi_month->y;
		/** Updates x and y, to start writting inside month box, at the left upper corner of next day box */
		x = PAN_BOX_BORDER + col * PAN_CAL_DAY_WIDTH;
		y = pi_month->y + pi_month->height + PAN_BOX_BORDER;
		/** While there are days to show, do work */
		for (day = 1; day <= days; day++)
			{
			FileData *fd;
			PanItem *pi_day;
			gint dx, dy;
			gint n = 0;
			gchar fake_path[20];
			/** Get day of the week ; string and number */
			dt = pan_date_to_time(year, month, day);

			/*
			 * Create a FileData entry that represents the given day.
			 * It does not correspond to any real file
			 */

			g_snprintf(fake_path, sizeof(fake_path), "//%04d-%02d-%02d", year, month, day);
			fd = file_data_new_simple(fake_path);
			fd->date = dt;
			/** Creates new pan item box that represents all the pictures in current considered day */
			pi_day = pan_item_box_new(pw, fd, x, y, PAN_CAL_DAY_WIDTH, PAN_CAL_DAY_HEIGHT,
						  PAN_CAL_DAY_BORDER,
						  PAN_CAL_DAY_COLOR, PAN_CAL_DAY_ALPHA,
						  PAN_CAL_DAY_BORDER_COLOR, PAN_CAL_DAY_ALPHA);
			pan_item_set_key(pi_day, "day");
			/** Computes gaps between days */
			dx = x + PAN_CAL_DOT_GAP * 2;
			dy = y + PAN_CAL_DOT_GAP * 2;
			/** Do we have pics that day to show? */
			fd = (work) ? work->data : NULL;
			/** While we have pics to show current considered day, do work */
			while (fd && pan_date_compare(fd->date, dt, PAN_DATE_LENGTH_DAY))
				{
				PanItem *pi;
				/** Pan item box to present a pic. No borders or alpha.*/
				pi = pan_item_box_new(pw, fd, dx, dy, PAN_CAL_DOT_SIZE, PAN_CAL_DOT_SIZE,
						      0,
						      PAN_CAL_DOT_COLOR, PAN_CAL_DOT_ALPHA,
						      0, 0, 0, 0);
				pan_item_set_key(pi, "dot");
				/** Updates 'dx' to position next picture . Same for 'dy'.*/
				dx += PAN_CAL_DOT_SIZE + PAN_CAL_DOT_GAP;
				if (dx + PAN_CAL_DOT_SIZE > pi_day->x + pi_day->width - PAN_CAL_DOT_GAP * 2)
					{
					dx = x + PAN_CAL_DOT_GAP * 2;
					dy += PAN_CAL_DOT_SIZE + PAN_CAL_DOT_GAP;
					}
				if (dy + PAN_CAL_DOT_SIZE > pi_day->y + pi_day->height - PAN_CAL_DOT_GAP * 2)
					{
					/* must keep all dots within respective day even if it gets ugly */
					dy = y + PAN_CAL_DOT_GAP * 2;
					}
				/** Adittionally put a thumbnail*/
				if (n<pw->nthumbnails)
				{
				pi_image = pan_item_thumb_new(pw, fd, pi_day->x , pi_day->y);
			

				pan_item_adjust_size_to_item(pi_image,pi_day);
				}
				/** Updates numbers of pics found. */
				n++;
			
				work = work->next;
				fd = (work) ? work->data : NULL;
				}
			/** If we found any pic that day, do work */
			if (n > 0)
				{
				PanItem *pi;
				/** It computes intensity of color, depending of how many pics we found that day.
				    It will be darker if there are many pics, and lighter otherwise.*/
				pi_day->color_r = MAX(pi_day->color_r - 61 - n * 3, 80);
				pi_day->color_g = pi_day->color_r;
				/** */
				buf = g_strdup_printf("( %d )", n);
				pi = pan_item_text_new(pw, x, y, buf, PAN_TEXT_ATTR_NONE,
						       PAN_TEXT_BORDER_SIZE,
						       PAN_CAL_DAY_TEXT_COLOR, 255);
				g_free(buf);
				/** Positionates numbers of pictures found*/
				pi->x = pi_day->x + (pi_day->width - pi->width) / 2;
				pi->y = pi_day->y + (pi_day->height - pi->height) / 2;

		
				}
		
			buf = g_strdup_printf("%d", day);
			pan_item_text_new(pw, x + 4, y + 4, buf, PAN_TEXT_ATTR_BOLD | PAN_TEXT_ATTR_HEADING,
					  PAN_TEXT_BORDER_SIZE,
					  PAN_CAL_DAY_TEXT_COLOR, 255);
			g_free(buf);

			/** Updates width and height to the right bottom of current pan item box*/
			pan_item_size_coordinates(pi_day, PAN_BOX_BORDER, width, height);

			col++;
			/** If we have more than 6 columns, reset it. If not, update 'x' with next x for next day */
			if (col > 6)
				{
				col = 0;
				row++;
				x = PAN_BOX_BORDER;
				y += PAN_CAL_DAY_HEIGHT;
				}
			else
				{
				x += PAN_CAL_DAY_WIDTH;
				}
			}
	
		if (col > 0) y += PAN_CAL_DAY_HEIGHT;
		y += PAN_BOX_BORDER * 2;
		/** If month > 12; consider next year*/
		month ++;
		if (month > 12)
			{
			year++;
			month = 1;
			}
		}
	/** Finally, update width and height */
	*width += grid;
	*height = MAX(*height, grid + PAN_BOX_BORDER * 2 * 2);

	g_list_free(list);
  
}
void pan_cronos_compute(PanWindow *pw,FileData *dir_fd,gint *width,gint *height)
{
       if (pw->depth == PAN_DATE_LENGTH_DAY)
       {
          pan_cronos_calendar_compute(pw,dir_fd,width,height);
	  return;
       }
    

	GList *list;
     	GList *work;
	gint x, y;
	time_t tc;
	gint count;
	gint day_max;
	gint day_width;
	gint day_height;
	gint grid;
	depth_date = 0;
	gint month = 0;
	gint end_year = 0;
	gint end_month = 0;
	current_depth_date = 0;
	gint current_list_position = 0;
	DEBUG_1("PAN CRONOS----------------")     ;
        list = pan_list_tree(dir_fd, SORT_NONE, TRUE, pw->ignore_symlinks);
	/** If cache is enabled and exif data; sort it BY NAME, and sync with new list */
	if (pw->cache_list && pw->exif_date_enable)
		{
		pw->cache_list = pan_cache_sort(pw->cache_list, SORT_NAME, TRUE);
		list = filelist_sort(list, SORT_NAME, TRUE);
		pan_cache_sync_date(pw, list);
		}
        /** Now sync it by time , to get earlier pics at first of the list */
	pw->cache_list = pan_cache_sort(pw->cache_list, SORT_TIME, TRUE);
	list = filelist_sort(list, SORT_TIME, TRUE);

	 
       if (pw->depth == 99)
       {
	 view_window_new_from_list_position(list,pw->cronos_click->fd);
	 return;
       }
	work = list;
		while (work)
		{
		FileData *fd;

		fd = work->data;
		work = work->next;
		/** Compares new considered file date with day 'tc'. If tc=fd->date ; then increments the total
		    amount of pics found for that day; and updates day_max if that day has more pics than
		    all the days considered since now.If not, it updates 'tc' with the new day, and resets counting */
		if (!pan_date_compare(fd->date, tc, pw->depth))
			{
			count = 0;
			tc = fd->date;
			}
		else
			{
			count++;
			if (day_max < count) day_max = count;
			}
		}

	DEBUG_1("biggest day contains %d images", day_max);
	grid = (gint)(sqrt((gdouble)day_max) + 0.5) * (PAN_THUMB_SIZE + PAN_SHADOW_OFFSET * 2 + PAN_THUMB_GAP);
	day_width = MAX(PAN_CAL_DAY_WIDTH, grid);
	day_height = MAX(PAN_CAL_DAY_HEIGHT, grid);


	/** Now we have all the diferent dat depth's. Now we can create the  boxes */
	    /** Create "year" boxs*/
	   /** Get first year*/	  
	
	   if (list)
		{
		FileData *fd = list->data;

		depth_date = pan_date_value(fd->date, PAN_DATE_LENGTH_YEAR);
		}
//           DEBUG_1("First year is: %d",year);
	   *width = PAN_BOX_BORDER * 2;
   	   *height = PAN_BOX_BORDER * 2;

	   x = PAN_BOX_BORDER;
	   y = PAN_BOX_BORDER;

	   work = list;
	                                         
	   /** If depyh*/
	   while (work)
	   {
	      FileData *fd;
	      fd = work->data;
	      work = work->next;
   	      depth_date = pan_date_value(fd->date, pw->depth);
	      if (pan_cronos_more_to_show(pw,fd->date))
	      {
		/** we have to create a new box */
			/** New pan item : month */
		PanItem *pi_month;
		PanItem *pi_text;
		PanItem *pi_image;
		gint day;
		gint days;
		gint col;
		gint row;
		time_t dt;
		gchar *buf;
		dt = fd->date;
		DEBUG_1("Placing x and y in: %d , %d ",x,y);
		pi_month = pan_item_box_new(pw, NULL, x, y, PAN_CAL_DAY_WIDTH * 2, PAN_CAL_DAY_HEIGHT *2,
					    PAN_CAL_MONTH_BORDER,
					    PAN_CAL_MONTH_COLOR, PAN_CAL_MONTH_ALPHA,
					    PAN_CAL_MONTH_BORDER_COLOR, PAN_CAL_MONTH_ALPHA);
	
		buf = pan_date_value_string(dt, pw->depth);
		/** Date text */
		pi_text = pan_item_text_new(pw, x, y, buf,
					    PAN_TEXT_ATTR_BOLD | PAN_TEXT_ATTR_HEADING,
					    PAN_TEXT_BORDER_SIZE,
					    PAN_CAL_MONTH_TEXT_COLOR, 255);
		g_free(buf);
		/** pi_text x and y position.To make it start at left upper corner of month box. */
		pi_text->x = pi_month->x + (pi_month->width - pi_text->width) / 2;
//		if (pw->depth == PAN_DATE_LENGTH_EXACT) pi_image = pan_item_image_new (pw,fd,pi_month->x,pi_month->y + PAN_CAL_MONTH_BORDER,10,10);
		pi_image = pan_item_thumb_new(pw, fd, pi_month->x , pi_month->y + PAN_CAL_MONTH_BORDER);

		pan_item_size_by_item(pi_month,pi_image,PAN_CAL_MONTH_BORDER);
		pan_item_center_by_item(pi_month,pi_image);
	 	pan_item_size_coordinates(pi_month, PAN_BOX_BORDER, width, height);

		/** Update x */
		x += pi_month->width + PAN_THUMB_GAP;
		current_depth_date = depth_date;
	
	      
              }
	      /** Update list postiion. We need this for further slideshow start */
	      current_list_position++;

	}
	*width += grid;
	*height = MAX(*height, grid + PAN_BOX_BORDER * 2 * 2);

	if (pw->depth == PAN_DATE_LENGTH_EXACT) pw->current_list_position = current_list_position -1;

	g_list_free(list);



}
