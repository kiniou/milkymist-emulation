/*
 * Milkymist Emulation Libraries
 * Copyright (C) 2007, 2008, 2009 Sebastien Bourdeauducq
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <hal/tmu.h>

void tmu_init()
{
	printf("TMU: software based emulation\n");
}

void tmu_isr()
{
	printf("tmu_isr() should not be called in emulation mode\n");
}

struct interpolator {
	int i;		/* increment/decrement */
	int q, r;	/* quotient and remainder */
	int e;		/* error accumulator */
	int v;		/* current value */
	int n;		/* number of steps */
};

static void interpolator_init(struct interpolator *i,
	int n, int y1, int y2)
{
	int dy;
	
	dy = y2 - y1;
	i->i = dy >= 0;
	if(dy < 0) dy = -dy;
	i->q = dy / n;
	i->r = dy % n;
	i->e = 0;
	i->v = y1;
	i->n = n;
}

static void interpolator_step(struct interpolator *i)
{
	if(i->i)
		i->v += i->q;
	else
		i->v -= i->q;
	i->e += i->r;
	if(i->e > (i->n/2)) {
		if(i->i)
			i->v++;
		else
			i->v--;
		i->e -= i->n;
	}
}

static void point(struct tmu_td *td, int sx, int sy, int x, int y)
{
	unsigned int c;
	
	if(sx < 0) sx = 0;
	if(sy < 0) sy = 0;
	if(sx >= td->srchres) sx = td->srchres-1;
	if(sy >= td->srcvres) sy = td->srcvres-1;
	
	if(x < 0) return;
	if(y < 0) return;
	if(x >= td->dsthres) return;
	if(y >= td->dstvres) return;

	c = td->srcfbuf[td->srchres*sy+sx];
	if((td->flags & TMU_CTL_CHROMAKEY) && (c == td->chromakey)) return;
	c = (c*td->brightness) >> 6;
	td->dstfbuf[td->dsthres*y+x] = c;
}

static void hinterpolate(struct tmu_td *td, int y,
	int x1, int sx1, int sy1,
	int x2, int sx2, int sy2)
{
	struct interpolator sx, sy;
	int x, n;
	
	n = x2-x1;
	interpolator_init(&sx, n, sx1, sx2);
	interpolator_init(&sy, n, sy1, sy2);
	point(td, sx.v, sy.v, x1, y);
	for(x=x1+1;x<=x2;x++) {
		interpolator_step(&sx);
		interpolator_step(&sy);
		point(td, sx.v, sy.v, x, y);
	}
}

static void vinterpolate(struct tmu_td *td,
	struct tmu_vertex sa, struct tmu_vertex sb, struct tmu_vertex sc,
	struct tmu_vertex da, struct tmu_vertex db, struct tmu_vertex dc)
{
	int y;
	struct interpolator x1, x2; /* start and end of the scanline (dest) */
	struct interpolator sx1, sx2, sy1, sy2; /* src coordinates at start and end of scanline */

	if(db.x < dc.x) {
		if(db.y == da.y) {
			hinterpolate(td, db.y,
				da.x, sa.x, sa.y,
				db.x, sb.x, sb.y);
		} else {
			interpolator_init(&x1, dc.y-da.y, da.x, dc.x);
			interpolator_init(&x2, db.y-da.y, da.x, db.x);
			interpolator_init(&sx1, dc.y-da.y, sa.x, sc.x);
			interpolator_init(&sx2, db.y-da.y, sa.x, sb.x);
			interpolator_init(&sy1, dc.y-da.y, sa.y, sc.y);
			interpolator_init(&sy2, db.y-da.y, sa.y, sb.y);
			for(y=da.y;y<db.y;y++) {
				hinterpolate(td, y,
					x1.v, sx1.v, sy1.v,
					x2.v, sx2.v, sy2.v);
				interpolator_step(&x1);
				interpolator_step(&x2);
				interpolator_step(&sx1);
				interpolator_step(&sx2);
				interpolator_step(&sy1);
				interpolator_step(&sy2);
			}
		}
		if(db.y != dc.y) {
			interpolator_init(&x2, dc.y-db.y, db.x, dc.x);
			interpolator_init(&sx2, dc.y-db.y, sb.x, sc.x);
			interpolator_init(&sy2, dc.y-db.y, sb.y, sc.y);
			for(y=db.y;y<=dc.y;y++) {
				hinterpolate(td, y,
					x1.v, sx1.v, sy1.v,
					x2.v, sx2.v, sy2.v);
				interpolator_step(&x1);
				interpolator_step(&x2);
				interpolator_step(&sx1);
				interpolator_step(&sx2);
				interpolator_step(&sy1);
				interpolator_step(&sy2);
			}
		}
	} else {
		if(db.y == da.y) {
			hinterpolate(td, db.y,
				da.x, sa.x, sa.y,
				db.x, sb.x, sb.y);
		} else {
			interpolator_init(&x1, db.y-da.y, da.x, db.x);
			interpolator_init(&x2, dc.y-da.y, da.x, dc.x);
			interpolator_init(&sx1, db.y-da.y, sa.x, sb.x);
			interpolator_init(&sx2, dc.y-da.y, sa.x, sc.x);
			interpolator_init(&sy1, db.y-da.y, sa.y, sb.y);
			interpolator_init(&sy2, dc.y-da.y, sa.y, sc.y);
			for(y=da.y;y<dc.y;y++) {
				hinterpolate(td, y,
					x1.v, sx1.v, sy1.v,
					x2.v, sx2.v, sy2.v);
				interpolator_step(&x1);
				interpolator_step(&x2);
				interpolator_step(&sx1);
				interpolator_step(&sx2);
				interpolator_step(&sy1);
				interpolator_step(&sy2);
			}
		}
		if(dc.y != db.y) {
			interpolator_init(&x2, db.y-dc.y, dc.x, db.x);
			interpolator_init(&sx2, db.y-dc.y, sc.x, sb.x);
			interpolator_init(&sy2, db.y-dc.y, sc.y, sb.y);
			for(y=dc.y;y<=db.y;y++) {
				hinterpolate(td, y,
					x1.v, sx1.v, sy1.v,
					x2.v, sx2.v, sy2.v);
				interpolator_step(&x1);
				interpolator_step(&x2);
				interpolator_step(&sx1);
				interpolator_step(&sx2);
				interpolator_step(&sy1);
				interpolator_step(&sy2);
			}
		}
	}
}

static void triangle(struct tmu_td *td,
	struct tmu_vertex s1, struct tmu_vertex s2, struct tmu_vertex s3,
	struct tmu_vertex d1, struct tmu_vertex d2, struct tmu_vertex d3)
{
	struct tmu_vertex sa, sb, sc;
	struct tmu_vertex da, db, dc;
	
	/* Sort the vertices so that da.y <= db.y <= dc.y */
	if((d1.y <= d2.y) && (d1.y <= d3.y)) {
		/* A=1 */
		sa = s1;
		da = d1;
		if(d2.y <= d3.y) {
			/* B=2 C=3 */
			sb = s2;
			db = d2;
			sc = s3;
			dc = d3;
		} else {
			/* B=3 C=2 */
			sb = s3;
			db = d3;
			sc = s2;
			dc = d2;
		}
	} else if((d2.y <= d3.y) && (d2.y <= d1.y)) {
		/* A=2 */
		sa = s2;
		da = d2;
		if(d1.y <= d3.y) {
			/* B=1 C=3 */
			sb = s1;
			db = d1;
			sc = s3;
			dc = d3;
		} else {
			/* B=3 C=1 */
			sb = s3;
			db = d3;
			sc = s1;
			dc = d1;
		}
	} else {
		/* A=3 */
		sa = s3;
		da = d3;
		if(d1.y <= d2.y) {
			/* B=1 C=2 */
			sb = s1;
			db = d1;
			sc = s2;
			dc = d2;
		} else {
			/* B=2 C=1 */
			sb = s2;
			db = d2;
			sc = s1;
			dc = d1;
		}
	}
	
	vinterpolate(td,
		sa, sb, sc,
		da, db, dc);
}

int tmu_submit_task(struct tmu_td *td)
{
	int x, y;
	
	if(td->hmeshlast >= TMU_MESH_MAXSIZE) {
		printf("tmu_submit_task() called with hmeshsize too high!\n");
		return 0;
	}
	if(td->vmeshlast >= TMU_MESH_MAXSIZE) {
		printf("tmu_submit_task() called with vmeshsize too high!\n");
		return 0;
	}
	
	for(y=0;y<td->vmeshlast;y++)
		for(x=0;x<td->hmeshlast;x++) {
#define M(_m, _x, _y) (_m)[TMU_MESH_MAXSIZE*(_y)+(_x)]
			triangle(td,
				M(td->srcmesh, x, y), M(td->srcmesh, x+1, y), M(td->srcmesh, x, y+1),
				M(td->dstmesh, x, y), M(td->dstmesh, x+1, y), M(td->dstmesh, x, y+1)
			);
			triangle(td,
				M(td->srcmesh, x+1, y+1), M(td->srcmesh, x+1, y), M(td->srcmesh, x, y+1),
				M(td->dstmesh, x+1, y+1), M(td->dstmesh, x+1, y), M(td->dstmesh, x, y+1)
			);
#undef M
		}
	
	td->callback(td);
	
	return 1;
}
