/********************************************************************************
 * Copyright (C) 2007 by Prakash Punnoor                                        *
 * prakash@punnoor.de                                                           *
 *                                                                              *
 * This library is free software; you can redistribute it and/or                *
 * modify it under the terms of the GNU Lesser General Public                   *
 * License as published by the Free Software Foundation; either                 *
 * version 2 of the License                                                     *
 *                                                                              *
 * This library is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU            *
 * Lesser General Public License for more details.                              *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public             *
 * License along with this library; if not, write to the Free Software          *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA *
 ********************************************************************************/
using System;
using System.Runtime.InteropServices;

namespace Aften
{
	public static class Utility
	{
		[DllImport( "aften.dll" )]
		private static extern IntPtr aften_get_version(); // returns string not on heap!
		
		[DllImport( "aften.dll" )]
		private static extern int aften_wav_channels_to_acmod(
			int ch, uint chmask, out AudioCodingMode acmod, out bool lfe);

		[DllImport( "aften.dll" )]
		private static extern int aften_remap_wav_to_a52(
			float[] samples, int n, int ch, A52SampleFormat fmt, AudioCodingMode acmod);

		[DllImport( "aften.dll" )]
		private static extern int aften_remap_mpeg_to_a52(
			float[] samples, int n, int ch, A52SampleFormat fmt, AudioCodingMode acmod);

		[DllImport( "aften.dll" )]
		private static extern FloatType aften_get_float_type();


		public static string GetVersion()
		{
			return Marshal.PtrToStringAuto( aften_get_version() );
		}

		public static void WaveChannelsToAcmod(
			int channels, uint channelMask, out AudioCodingMode audioCodingMode, out bool hasLfe)
		{
			if(aften_wav_channels_to_acmod(
				channels, channelMask, out audioCodingMode, out hasLfe ) != 0)
				throw new InvalidOperationException( "channels or channelMask are not valid");
		}

		public static void RemapWaveToA52(
			float[] samples, int samplesCount, int channels, A52SampleFormat format, AudioCodingMode audioCodingMode)
		{
			aften_remap_wav_to_a52( samples, samplesCount, channels, format, audioCodingMode );
		}

		public static void RemapMpegToA52(
			float[] samples, int samplesCount, int channels, A52SampleFormat format, AudioCodingMode audioCodingMode)
		{
			aften_remap_mpeg_to_a52( samples, samplesCount, channels, format, audioCodingMode );
		}

		public static FloatType GetFloatType()
		{
			return aften_get_float_type();
		}
	}
}