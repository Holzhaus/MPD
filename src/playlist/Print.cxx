/*
 * Copyright 2003-2017 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include "Print.hxx"
#include "PlaylistAny.hxx"
#include "PlaylistSong.hxx"
#include "SongEnumerator.hxx"
#include "SongPrint.hxx"
#include "DetachedSong.hxx"
#include "fs/Traits.hxx"
#include "thread/Mutex.hxx"
#include "thread/Cond.hxx"
#include "Partition.hxx"
#include "Instance.hxx"

static void
playlist_provider_print(Response &r,
			const SongLoader &loader,
			const char *uri,
			SongEnumerator &e, bool detail) noexcept
{
	const std::string base_uri = uri != nullptr
		? PathTraitsUTF8::GetParent(uri)
		: std::string(".");

	std::unique_ptr<DetachedSong> song;
	while ((song = e.NextSong()) != nullptr) {
		if (playlist_check_translate_song(*song, base_uri.c_str(),
						  loader) &&
		    detail)
			song_print_info(r, *song);
		else
			/* fallback if no detail was requested or no
			   detail was available */
			song_print_uri(r, *song);
	}
}

bool
playlist_file_print(Response &r, Partition &partition,
		    const SongLoader &loader,
		    const char *uri, bool detail)
{
	Mutex mutex;
	Cond cond;

#ifndef ENABLE_DATABASE
	(void)partition;
#endif

	auto playlist = playlist_open_any(uri,
#ifdef ENABLE_DATABASE
					  partition.instance.storage,
#endif
					  mutex, cond);
	if (playlist == nullptr)
		return false;

	playlist_provider_print(r, loader, uri, *playlist, detail);
	return true;
}
