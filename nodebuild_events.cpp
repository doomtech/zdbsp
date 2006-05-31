/*
    A red-black tree implementation for building minisegs.
    Copyright (C) 2002,2003 Randy Heit

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "zdbsp.h"
#include "nodebuild.h"

FEventTree::FEventTree ()
: Root (&Nil), Spare (NULL)
{
	memset (&Nil, 0, sizeof(Nil));
	Nil.Color = FEvent::BLACK;
}

FEventTree::~FEventTree ()
{
	FEvent *probe;

	DeleteAll ();
	probe = Spare;
	while (probe != NULL)
	{
		FEvent *next = probe->Left;
		delete probe;
		probe = next;
	}
}

void FEventTree::DeleteAll ()
{
	DeletionTraverser (Root);
	Root = &Nil;
}

void FEventTree::DeletionTraverser (FEvent *node)
{
	if (node != &Nil && node != NULL)
	{
		DeletionTraverser (node->Left);
		DeletionTraverser (node->Right);
		node->Left = Spare;
		Spare = node;
	}
}

void FEventTree::LeftRotate (FEvent *x)
{
	FEvent *y = x->Right;
	x->Right = y->Left;
	if (y->Left != &Nil)
	{
		y->Left->Parent = x;
	}
	y->Parent = x->Parent;
	if (x->Parent == &Nil)
	{
		Root = y;
	}
	else if (x == x->Parent->Left)
	{
		x->Parent->Left = y;
	}
	else
	{
		x->Parent->Right = y;
	}
	y->Left = x;
	x->Parent = y;
}

void FEventTree::RightRotate (FEvent *x)
{
	FEvent *y = x->Left;
	x->Left = y->Right;
	if (y->Right != &Nil)
	{
		y->Right->Parent = x;
	}
	y->Parent = x->Parent;
	if (x->Parent == &Nil)
	{
		Root = y;
	}
	else if (x == x->Parent->Left)
	{
		x->Parent->Left = y;
	}
	else
	{
		x->Parent->Right = y;
	}
	y->Right = x;
	x->Parent = y;
}

FEvent *FEventTree::GetNewNode ()
{
	FEvent *node;

	if (Spare != NULL)
	{
		node = Spare;
		Spare = node->Left;
	}
	else
	{
		node = new FEvent;
	}
	return node;
}

void FEventTree::Insert (FEvent *z)
{
	FEvent *y = &Nil;
	FEvent *x = Root;

	while (x != &Nil)
	{
		y = x;
		if (z->Distance < x->Distance)
		{
			x = x->Left;
		}
		else
		{
			x = x->Right;
		}
	}
	z->Parent = y;
	if (y == &Nil)
	{
		Root = z;
	}
	else if (z->Distance < y->Distance)
	{
		y->Left = z;
	}
	else
	{
		y->Right = z;
	}
	z->Left = &Nil;
	z->Right = &Nil;
//	printf ("Inserted      distance %g, vertex %d, seg %d\n",
//		sqrt(z->Distance/4294967296.0), z->Info.Vertex, z->Info.FrontSeg);

	z->Color = FEvent::RED;
	while (z != Root && z->Parent->Color == FEvent::RED)
	{
		if (z->Parent == z->Parent->Parent->Left)
		{
			y = z->Parent->Parent->Right;
			if (y->Color == FEvent::RED)
			{
				z->Parent->Color = FEvent::BLACK;
				y->Color = FEvent::BLACK;
				z->Parent->Parent->Color = FEvent::RED;
				z = z->Parent->Parent;
			}
			else
			{
				if (z == z->Parent->Right)
				{
					z = z->Parent;
					LeftRotate (z);
				}
				z->Parent->Color = FEvent::BLACK;
				z->Parent->Parent->Color = FEvent::RED;
				RightRotate (z->Parent->Parent);
			}
		}
		else
		{
			y = z->Parent->Parent->Left;
			if (y->Color == FEvent::RED)
			{
				z->Parent->Color = FEvent::BLACK;
				y->Color = FEvent::BLACK;
				z->Parent->Parent->Color = FEvent::RED;
				z = z->Parent->Parent;
			}
			else
			{
				if (z == z->Parent->Left)
				{
					z = z->Parent;
					RightRotate (z);
				}
				z->Parent->Color = FEvent::BLACK;
				z->Parent->Parent->Color = FEvent::RED;
				LeftRotate (z->Parent->Parent);
			}
		}
	}
	Root->Color = FEvent::BLACK;
}

void FEventTree::Delete (FEvent *z)
{
	FEvent *x, *y;

	if (z->Left == &Nil || z->Right == &Nil)
	{
		y = z;
	}
	else
	{
		y = Successor (z);
	}
	if (y->Left != &Nil)
	{
		x = y->Left;
	}
	else
	{
		x = y->Right;
	}
	x->Parent = y->Parent;
	if (y->Parent == &Nil)
	{
		Root = x;
	}
	else if (y == y->Parent->Left)
	{
		y->Parent->Left = x;
	}
	else
	{
		y->Parent->Right = x;
	}
	if (y != z)
	{
		z->Distance = y->Distance;
		z->Info = y->Info;
	}
	if (y->Color == FEvent::BLACK)
	{
		DeleteFixUp (x);
	}

	y->Left = Spare;
	Spare = y;
}

void FEventTree::DeleteFixUp (FEvent *x)
{
	FEvent *w;

	while (x != Root && x->Color == FEvent::BLACK)
	{
		if (x == x->Parent->Left)
		{
			w = x->Parent->Right;
			if (w->Color == FEvent::RED)
			{
				w->Color = FEvent::BLACK;
				x->Parent->Color = FEvent::RED;
				LeftRotate (x->Parent);
				w = x->Parent->Right;
			}
			if (w->Left->Color == FEvent::BLACK && w->Right->Color == FEvent::BLACK)
			{
				w->Color = FEvent::RED;
				x = x->Parent;
			}
			else
			{
				if (w->Right->Color == FEvent::BLACK)
				{
					w->Left->Color = FEvent::BLACK;
					w->Color = FEvent::RED;
					RightRotate (w);
					w = x->Parent->Right;
				}
				w->Color = x->Parent->Color;
				x->Parent->Color = FEvent::BLACK;
				w->Right->Color = FEvent::BLACK;
				LeftRotate (x->Parent);
				x = Root;
			}
		}
		else
		{
			w = x->Parent->Left;
			if (w->Color == FEvent::RED)
			{
				w->Color = FEvent::BLACK;
				x->Parent->Color = FEvent::RED;
				RightRotate (x->Parent);
				w = x->Parent->Left;
			}
			if (w->Right->Color == FEvent::BLACK && w->Left->Color == FEvent::BLACK)
			{
				w->Color = FEvent::RED;
				x = x->Parent;
			}
			else
			{
				if (w->Left->Color == FEvent::BLACK)
				{
					w->Right->Color = FEvent::BLACK;
					w->Color = FEvent::RED;
					LeftRotate (w);
					w = x->Parent->Left;
				}
				w->Color = x->Parent->Color;
				x->Parent->Color = FEvent::BLACK;
				w->Left->Color = FEvent::BLACK;
				RightRotate (x->Parent);
				x = Root;
			}
		}
	}
}

FEvent *FEventTree::Successor (FEvent *event) const
{
	if (event->Right != &Nil)
	{
		event = event->Right;
		while (event->Left != &Nil)
		{
			event = event->Left;
		}
		return event;
	}
	else
	{
		FEvent *y = event->Parent;
		while (y != &Nil && event == y->Right)
		{
			event = y;
			y = y->Parent;
		}
		return y;
	}
}

FEvent *FEventTree::Predecessor (FEvent *event) const
{
	if (event->Left != &Nil)
	{
		event = event->Left;
		while (event->Right != &Nil)
		{
			event = event->Right;
		}
		return event;
	}
	else
	{
		FEvent *y = event->Parent;
		while (y != &Nil && event == y->Left)
		{
			event = y;
			y = y->Parent;
		}
		return y;
	}
}

FEvent *FEventTree::FindEvent (double key) const
{
	FEvent *node = Root;

	while (node != &Nil)
	{
		if (node->Distance == key)
		{
			return node;
		}
		else if (node->Distance > key)
		{
			node = node->Left;
		}
		else
		{
			node = node->Right;
		}
	}
	return NULL;
}

FEvent *FEventTree::GetMinimum ()
{
	FEvent *node = Root;

	if (node == &Nil)
	{
		return NULL;
	}
	while (node->Left != &Nil)
	{
		node = node->Left;
	}
	return node;
}

void FEventTree::PrintTree (const FEvent *event) const
{
	if (event->Left != &Nil)
	{
		PrintTree (event->Left);
	}
	printf (" Color %s, distance %g, vertex %d, seg %lu\n", event->Color ? "BLACK" : " RED ",
		sqrt(event->Distance/4294967296.0), event->Info.Vertex, event->Info.FrontSeg);
	if (event->Right != &Nil)
	{
		PrintTree (event->Right);
	}
}
