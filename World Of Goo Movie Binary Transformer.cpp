#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <Windows.h>
#include <atlconv.h>
using namespace std;

#define soundStr(x) ((x.soundStrIdx)?(string("(")+to_string(x.soundStrIdx)+string(")")+(x.soundString)):string("0"))

namespace oldFormat {

	enum TransformType {
		XFORM_SCALE = 0,
		XFORM_ROTATE = 1,
		XFORM_TRANSLATE = 2
	};

	enum InterpolationType {
		INTERPOLATION_NONE = 0,
		INTERPOLATION_LINEAR = 1
	};

	enum ActorType
	{
		eActorType_Image = 0,
		eActorType_Text = 1
	};

	enum AlignmentH
	{
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2
	};

	enum AlignmentV
	{
		ALIGN_TOP = 0,
		ALIGN_MIDDLE = 1,
		ALIGN_BOTTOM = 2
	};

	struct keyframe
	{
		bool isValid = false;
		float x = 0.f;
		float y = 0.f;
		float angle = 0.f;
		int alpha = 0;
		int color = 0;
		int nextFrameIndex = 0;
		int soundStrIdx = 0;
		InterpolationType interpolation = InterpolationType::INTERPOLATION_LINEAR;
		string soundString = "";
	};

	struct keyframeIO
	{
		float x = 0.f;
		float y = 0.f;
		float angle = 0.f;
		int alpha = 0;
		int color = 0;
		int nextFrameIndex = 0;
		int soundStrIdx = 0;
		InterpolationType interpolation = InterpolationType::INTERPOLATION_LINEAR;
	};

	struct BinImageAnimation
	{
		int animOffset = 0;

		int mHasColor = 0;
		int mHasAlpha = 0;
		int mHasSound = 0;
		int mHasTransform = 0;
		int mNumTransforms = 0;
		int mNumFrames = 0;

		int transformTypeOffset = 0;
		int frameTimesOffset = 0;
		int xformFramesOffset = 0;
		int alphaFramesOffset = 0;
		int colorFramesOffset = 0;
		int soundFramesOffset = 0;
		int stringTableOffset = 0;

		TransformType* mTransformTypes;
		int TransFrameOffset[3] = { 0 };
		int* AlphaOffset;
		int* ColorOffset;
		int* SoundOffset;
		int* TransSingleFrameOffset[3];
		float* mFrameTimes;
		keyframe* mXformFrames[3];
		keyframe* mAlphaFrames;
		keyframe* mColorFrames;
		keyframe* mSoundFrames;
	};

	struct AnimIO {
		int mHasColor = 0;
		int mHasAlpha = 0;
		int mHasSound = 0;
		int mHasTransform = 0;
		int mNumTransforms = 0;
		int mNumFrames = 0;

		int transformTypeOffset = 0;
		int frameTimesOffset = 0;
		int xformFramesOffset = 0;
		int alphaFramesOffset = 0;
		int colorFramesOffset = 0;
		int soundFramesOffset = 0;
		int stringTableOffset = 0;
	};

	struct BinActor
	{
		ActorType mType = ActorType::eActorType_Image;
		int mImageStrIdx = 0;
		string ImageStr = "";
		int mLabelTextStrIdx = 0;
		string LabelTextStr = "";
		int mFontStrIdx = 0;
		string FontStr = "";
		float mLabelMaxWidth = 0.f;
		float mLabelWrapWidth = 0.f;
		AlignmentH mLabelJustification = AlignmentH::ALIGN_CENTER;
		float mDepth = 0.f;
	};

	struct ActorIO
	{
		ActorType mType = ActorType::eActorType_Image;
		int mImageStrIdx = 0;
		int mLabelTextStrIdx = 0;
		int mFontStrIdx = 0;
		float mLabelMaxWidth = 0.f;
		float mLabelWrapWidth = 0.f;
		AlignmentH mLabelJustification = AlignmentH::ALIGN_CENTER;
		float mDepth = 0.f;
	};

	struct MovieIO {
		float length = 0.f;
		int numActors = 0;
		int actorsOffset = 0;
		int animsOffset = 0;
		int stringsOffset = 0;
	};

	struct BinMovie
	{
		float length = 0.f;
		int numActors = 0;
		BinActor* pActors;
		BinImageAnimation* pAnims;

		//inputed
		int actorsOffset = 0;
		int animsOffset = 0;
		int stringsOffset = 0;
	};

	string readStr(ifstream& stream, int offset) {
		stream.seekg(offset, ios::beg);
		char chr;
		string str = "";
		stream.read(&chr, sizeof(char));
		while ((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') || chr == '_' || chr == '$') {
			str += chr;
			stream.read(&chr, sizeof(char));
		}
		//printf("\t\tstring[%d - %d] = \"%s\"\n", offset, offset + 1 + str.length(), str.c_str());
		return str;
	}

	void writeStr(ofstream& stream, int offset, string str) {
		stream.seekp(offset, ios::beg);
		stream.write(str.c_str(), sizeof(char) * str.length());
	}

	void ReadAnim(ifstream& stream, int offset, BinImageAnimation& Anim) {
		AnimIO AnimIn;
		stream.seekg(offset, ios::beg);
		stream.read((char*)&AnimIn, sizeof(AnimIO));
		Anim.mHasColor = AnimIn.mHasColor;
		Anim.mHasAlpha = AnimIn.mHasAlpha;
		Anim.mHasSound = AnimIn.mHasSound;
		Anim.mHasTransform = AnimIn.mHasTransform;
		Anim.mNumTransforms = AnimIn.mNumTransforms;
		Anim.mNumFrames = AnimIn.mNumFrames;
		Anim.transformTypeOffset = AnimIn.transformTypeOffset;
		Anim.frameTimesOffset = AnimIn.frameTimesOffset;
		Anim.xformFramesOffset = AnimIn.xformFramesOffset;
		Anim.alphaFramesOffset = AnimIn.alphaFramesOffset;
		Anim.colorFramesOffset = AnimIn.colorFramesOffset;
		Anim.soundFramesOffset = AnimIn.soundFramesOffset;
		Anim.stringTableOffset = AnimIn.stringTableOffset;
		Anim.animOffset = offset;
		//printf("\tAnime string offset = %d\n", Anim.stringTableOffset);

		//get framTimes
		stream.seekg(Anim.animOffset + Anim.frameTimesOffset, ios::beg);
		Anim.mFrameTimes = new float[Anim.mNumFrames];
		for (int i = 0; i < Anim.mNumFrames; i++)
			stream.read((char*)(Anim.mFrameTimes + i), sizeof(float));

		keyframeIO keyIn;

		//get Transform Types
		stream.seekg(Anim.animOffset + Anim.transformTypeOffset, ios::beg);
		Anim.mTransformTypes = new TransformType[Anim.mNumTransforms];
		for (int i = 0; i < Anim.mNumTransforms; i++)
			stream.read((char*)(Anim.mTransformTypes + i), sizeof(TransformType));

		//get Transforms
		if (Anim.mHasTransform) {
			//get xformFramesOffset
			stream.seekg(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumTransforms; i++)
				stream.read((char*)(Anim.TransFrameOffset + i), sizeof(int));

			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++) {
				Anim.mXformFrames[i] = new keyframe[Anim.mNumFrames];
				Anim.TransSingleFrameOffset[i] = new int[Anim.mNumFrames];
				stream.seekg(Anim.animOffset + Anim.TransFrameOffset[i], ios::beg);
				for (int j = 0; j < Anim.mNumFrames; j++) {
					stream.read((char*)(Anim.TransSingleFrameOffset[i] + j), sizeof(int));
				}
				for (int j = 0; j < Anim.mNumFrames; j++) {
					if (Anim.TransSingleFrameOffset[i][j] == 0) {
						Anim.mXformFrames[i][j].isValid = false;
					}
					else {
						Anim.mXformFrames[i][j].isValid = true;
						stream.seekg(Anim.TransSingleFrameOffset[i][j] + Anim.animOffset, ios::beg);
						stream.read((char*)&keyIn, sizeof(keyframeIO));
						Anim.mXformFrames[i][j].alpha = keyIn.alpha;
						Anim.mXformFrames[i][j].angle = keyIn.angle;
						Anim.mXformFrames[i][j].color = keyIn.color;
						Anim.mXformFrames[i][j].interpolation = keyIn.interpolation;
						Anim.mXformFrames[i][j].nextFrameIndex = keyIn.nextFrameIndex;
						Anim.mXformFrames[i][j].soundStrIdx = keyIn.soundStrIdx;
						Anim.mXformFrames[i][j].x = keyIn.x;
						Anim.mXformFrames[i][j].y = keyIn.y;
					}
				}
				for (int j = 0; j < Anim.mNumFrames; j++)
					if (Anim.mXformFrames[i][j].soundStrIdx)
						Anim.mXformFrames[i][j].soundString = readStr(stream, Anim.mXformFrames[i][j].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
					else
						Anim.mXformFrames[i][j].soundString = "";
			}
		}


		//getAlpha
		if (Anim.mHasAlpha) {
			stream.seekg(Anim.animOffset + Anim.alphaFramesOffset, ios::beg);
			Anim.mAlphaFrames = new keyframe[Anim.mNumFrames];
			Anim.AlphaOffset = new int[Anim.mNumFrames];
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.read((char*)(Anim.AlphaOffset + i), sizeof(int));
			for (int i = 0; i < Anim.mNumFrames; i++) {
				if (Anim.AlphaOffset[i] == 0) {
					Anim.mAlphaFrames[i].isValid = false;
				}
				else {
					Anim.mAlphaFrames[i].isValid = true;
					stream.seekg(Anim.animOffset + Anim.AlphaOffset[i], ios::beg);
					stream.read((char*)&keyIn, sizeof(keyframeIO));
					Anim.mAlphaFrames[i].alpha = keyIn.alpha;
					Anim.mAlphaFrames[i].angle = keyIn.angle;
					Anim.mAlphaFrames[i].color = keyIn.color;
					Anim.mAlphaFrames[i].interpolation = keyIn.interpolation;
					Anim.mAlphaFrames[i].nextFrameIndex = keyIn.nextFrameIndex;
					Anim.mAlphaFrames[i].soundStrIdx = keyIn.soundStrIdx;
					Anim.mAlphaFrames[i].x = keyIn.x;
					Anim.mAlphaFrames[i].y = keyIn.y;
				}
			}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].soundStrIdx)
					Anim.mAlphaFrames[i].soundString = readStr(stream, Anim.mAlphaFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				else
					Anim.mAlphaFrames[i].soundString = "";

		}

		//getColor
		if (Anim.mHasColor) {
			stream.seekg(Anim.animOffset + Anim.colorFramesOffset, ios::beg);
			Anim.mColorFrames = new keyframe[Anim.mNumFrames];
			Anim.ColorOffset = new int[Anim.mNumFrames];
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.read((char*)(Anim.ColorOffset + i), sizeof(int));
			for (int i = 0; i < Anim.mNumFrames; i++) {
				if (Anim.ColorOffset[i] == 0) {
					Anim.mColorFrames[i].isValid = false;
				}
				else {
					Anim.mColorFrames[i].isValid = true;
					stream.seekg(Anim.animOffset + Anim.ColorOffset[i], ios::beg);
					stream.read((char*)&keyIn, sizeof(keyframeIO));
					Anim.mColorFrames[i].alpha = keyIn.alpha;
					Anim.mColorFrames[i].angle = keyIn.angle;
					Anim.mColorFrames[i].color = keyIn.color;
					Anim.mColorFrames[i].interpolation = keyIn.interpolation;
					Anim.mColorFrames[i].nextFrameIndex = keyIn.nextFrameIndex;
					Anim.mColorFrames[i].soundStrIdx = keyIn.soundStrIdx;
					Anim.mColorFrames[i].x = keyIn.x;
					Anim.mColorFrames[i].y = keyIn.y;
				}
			}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].soundStrIdx)
					Anim.mColorFrames[i].soundString = readStr(stream, Anim.mColorFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				else
					Anim.mColorFrames[i].soundString = "";

		}

		//getSound
		if (Anim.mHasSound) {
			stream.seekg(Anim.animOffset + Anim.soundFramesOffset, ios::beg);
			Anim.mSoundFrames = new keyframe[Anim.mNumFrames];
			Anim.SoundOffset = new int[Anim.mNumFrames];
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.read((char*)(Anim.SoundOffset + i), sizeof(int));
			for (int i = 0; i < Anim.mNumFrames; i++) {
				if (Anim.SoundOffset[i] == 0) {
					Anim.mSoundFrames[i].isValid = false;
				}
				else {
					Anim.mSoundFrames[i].isValid = true;
					stream.seekg(Anim.animOffset + Anim.SoundOffset[i], ios::beg);
					stream.read((char*)&keyIn, sizeof(keyframeIO));
					Anim.mSoundFrames[i].alpha = keyIn.alpha;
					Anim.mSoundFrames[i].angle = keyIn.angle;
					Anim.mSoundFrames[i].color = keyIn.color;
					Anim.mSoundFrames[i].interpolation = keyIn.interpolation;
					Anim.mSoundFrames[i].nextFrameIndex = keyIn.nextFrameIndex;
					Anim.mSoundFrames[i].soundStrIdx = keyIn.soundStrIdx;
					Anim.mSoundFrames[i].x = keyIn.x;
					Anim.mSoundFrames[i].y = keyIn.y;
				}
			}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].soundStrIdx)
					Anim.mSoundFrames[i].soundString = readStr(stream, Anim.mSoundFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				else
					Anim.mSoundFrames[i].soundString = "";
		}

	}

	void ReadMovie(ifstream& stream, int offset, BinMovie& Movie) {
		stream.seekg(offset, ios::beg);
		MovieIO MovieIn;

		stream.read((char*)&MovieIn, sizeof(MovieIO));

		Movie.length = MovieIn.length;
		Movie.numActors = MovieIn.numActors;
		Movie.pActors = new BinActor[MovieIn.numActors];
		Movie.pAnims = new BinImageAnimation[MovieIn.numActors];
		AnimIO* AnimIn = new AnimIO[MovieIn.numActors];
		ActorIO* ActorIn = new ActorIO[MovieIn.numActors];
		Movie.actorsOffset = MovieIn.actorsOffset;
		Movie.animsOffset = MovieIn.animsOffset;
		Movie.stringsOffset = MovieIn.stringsOffset;

		//printf("Movie string offset = %d\n", Movie.stringsOffset);

		//read Actors
		stream.seekg(offset + MovieIn.actorsOffset, ios::beg);
		for (int i = 0; i < MovieIn.numActors; i++) {
			stream.read((char*)(ActorIn + i), sizeof(ActorIO));
			Movie.pActors[i].mType = ActorIn[i].mType;
			Movie.pActors[i].mImageStrIdx = ActorIn[i].mImageStrIdx;
			Movie.pActors[i].mLabelTextStrIdx = ActorIn[i].mLabelTextStrIdx;
			Movie.pActors[i].mFontStrIdx = ActorIn[i].mFontStrIdx;
			Movie.pActors[i].mLabelMaxWidth = ActorIn[i].mLabelMaxWidth;
			Movie.pActors[i].mLabelWrapWidth = ActorIn[i].mLabelWrapWidth;
			Movie.pActors[i].mLabelJustification = ActorIn[i].mLabelJustification;
			Movie.pActors[i].mDepth = ActorIn[i].mDepth;
		}
		for (int i = 0; i < MovieIn.numActors; i++) {
			Movie.pActors[i].ImageStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mImageStrIdx);
			Movie.pActors[i].LabelTextStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mLabelTextStrIdx);
			Movie.pActors[i].FontStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mFontStrIdx);
		}

		//read Anim offset
		stream.seekg(offset + MovieIn.animsOffset, ios::beg);
		int* AnimOffset = new int[MovieIn.numActors];
		for (int i = 0; i < MovieIn.numActors; i++)
			stream.read((char*)(AnimOffset + i), sizeof(int));
		for (int i = 0; i < MovieIn.numActors; i++) {
			ReadAnim(stream, AnimOffset[i], Movie.pAnims[i]);
		}
	}

	string getType(BinActor& actor) {
		switch (actor.mType) {
		case 0:
			return string("IMAGE");
		case 1:
			return string("TEXT");
		default:
			return string("INVALID");
		}
	}

	string getTransformType(TransformType& Transform) {
		switch (Transform) {
		case 0:
			return string("SCALE");
		case 1:
			return string("ROTATE");
		case 2:
			return string("TRANSLATE");
		default:
			return string("INVALID");
		}
	}

	string getInterpolationType(InterpolationType& Int) {
		switch (Int) {
		case 0:
			return string("NONE");
		case 1:
			return string("LINEAR");
		default:
			return to_string(Int);
		}
	}

	string B2S(bool in) {
		return in ? "true" : "false";
	}

	template<typename _T>
	string to_string(_T x) {
		std::ostringstream ss;
		ss << x;
		return ss.str();
	}

	void print(BinActor& actor) {
		cout << "actor = BinActor{actorType=" << getType(actor) << ", imageStr='" << actor.ImageStr << "', labelStr='" << actor.LabelTextStr << "', fontStr='" << actor.FontStr << "', labelMaxWidth=" << actor.mLabelMaxWidth << ", labelWrapWidth=" << actor.mLabelWrapWidth << ", labelJustification=" << actor.mLabelJustification << ", depth=" << actor.mDepth << "}" << endl;
	}

	void print(BinImageAnimation& anim) {
		printf("binImageAnimOffset = %d\n", anim.animOffset);
		cout << "hasColor = " << B2S(anim.mHasColor) << endl;
		cout << "hasAlpha = " << B2S(anim.mHasAlpha) << endl;
		cout << "hasSound = " << B2S(anim.mHasSound) << endl;
		cout << "hasTransform = " << B2S(anim.mHasTransform) << endl;
		cout << "numTransforms = " << anim.mNumTransforms << endl;
		cout << "numFrames = " << anim.mNumFrames << endl;
		cout << "transformTypeOffset = " << anim.animOffset + anim.transformTypeOffset << endl;
		cout << "frameTimesOffset = " << anim.animOffset + anim.frameTimesOffset << endl;
		cout << "xformFramesOffset = " << anim.animOffset + anim.xformFramesOffset << endl;
		cout << "alphaFramesOffset = " << anim.animOffset + anim.alphaFramesOffset << endl;
		cout << "colorFramesOffset = " << anim.animOffset + anim.colorFramesOffset << endl;
		cout << "soundFramesOffset = " << anim.animOffset + anim.soundFramesOffset << endl;
		cout << "stringTableOffset = " << anim.animOffset + anim.stringTableOffset << endl;

		for (int i = 0; i < anim.mNumFrames; i++)
			cout << "frameTimes[" << i << "] = " << anim.mFrameTimes[i] << endl;

		if (anim.mHasTransform) {
			for (int i = 0; i < anim.mNumTransforms; i++)
				cout << "transformTypes[" << i << "] = " << getTransformType(anim.mTransformTypes[i]) << endl;
			for (int i = 0; i < anim.mNumTransforms; i++) {
				cout << "frameOffset = " << anim.TransFrameOffset[i] + anim.animOffset << endl;
				for (int j = 0; j < anim.mNumFrames; j++)
					if (anim.mXformFrames[i][j].isValid)
						cout << "( " << anim.TransSingleFrameOffset[i][j] << " )transformFrames[t=" << anim.mTransformTypes[i] << "," << getTransformType(anim.mTransformTypes[i]) << "][frame=" << j << "] = KeyFrame {x=" << anim.mXformFrames[i][j].x << ", y=" << anim.mXformFrames[i][j].y << ", angle=" << anim.mXformFrames[i][j].angle << ", alpha=" << anim.mXformFrames[i][j].alpha << ", color=" << anim.mXformFrames[i][j].color << ", nextFrameIndex=" << anim.mXformFrames[i][j].nextFrameIndex << ", soundString=" << soundStr(anim.mXformFrames[i][j]) << ", interpolationType=" << getInterpolationType(anim.mXformFrames[i][j].interpolation) << "}" << endl;
			}
		}
		if (anim.mHasAlpha)
			for (int i = 0; i < anim.mNumFrames; i++)
				if (anim.mAlphaFrames[i].isValid)
					cout << "alphaFrames[" << i << "] = KeyFrame {x=" << anim.mAlphaFrames[i].x << ", y=" << anim.mAlphaFrames[i].y << ", angle=" << anim.mAlphaFrames[i].angle << ", alpha=" << anim.mAlphaFrames[i].alpha << ", color=" << anim.mAlphaFrames[i].color << ", nextFrameIndex=" << anim.mAlphaFrames[i].nextFrameIndex << ", soundString=" << soundStr(anim.mAlphaFrames[i]) << ", interpolationType=" << getInterpolationType(anim.mAlphaFrames[i].interpolation) << "}" << endl;

		if (anim.mHasColor)
			for (int i = 0; i < anim.mNumFrames; i++)
				if (anim.mColorFrames[i].isValid)
					cout << "colorFrames[" << i << "] = KeyFrame {x=" << anim.mColorFrames[i].x << ", y=" << anim.mColorFrames[i].y << ", angle=" << anim.mColorFrames[i].angle << ", alpha=" << anim.mColorFrames[i].alpha << ", color=" << anim.mColorFrames[i].color << ", nextFrameIndex=" << anim.mColorFrames[i].nextFrameIndex << ", soundString=" << soundStr(anim.mColorFrames[i]) << ", interpolationType=" << getInterpolationType(anim.mColorFrames[i].interpolation) << "}" << endl;

		if (anim.mHasSound)
			for (int i = 0; i < anim.mNumFrames; i++)
				if (anim.mSoundFrames[i].isValid)
					cout << "soundFrames[" << i << "] = KeyFrame {x=" << anim.mSoundFrames[i].x << ", y=" << anim.mSoundFrames[i].y << ", angle=" << anim.mSoundFrames[i].angle << ", alpha=" << anim.mSoundFrames[i].alpha << ", color=" << anim.mSoundFrames[i].color << ", nextFrameIndex=" << anim.mSoundFrames[i].nextFrameIndex << ", soundString=" << soundStr(anim.mSoundFrames[i]) << ", interpolationType=" << getInterpolationType(anim.mSoundFrames[i].interpolation) << "}" << endl;

	}

	void print(BinMovie& movie) {
		printf("length = %f\n", movie.length);
		printf("numActors = %d\n", movie.numActors);
		printf("actorsOffset = %d\n", movie.actorsOffset);
		printf("animsOffset = %d\n", movie.animsOffset);
		printf("stringsOffset = %d\n", movie.stringsOffset);
		for (int i = 0; i < movie.numActors; i++) {
			printf("\n== ACTOR %d ==\n\n", i);
			print(movie.pActors[i]);
			print(movie.pAnims[i]);
		}
	}

	void output(ofstream& stream, int offset, BinImageAnimation& Anim) {
		AnimIO AnimOut;
		Anim.animOffset = offset;
		AnimOut.mHasColor = Anim.mHasColor;
		AnimOut.mHasAlpha = Anim.mHasAlpha;
		AnimOut.mHasSound = Anim.mHasSound;
		AnimOut.mHasTransform = Anim.mHasTransform;
		AnimOut.mNumTransforms = Anim.mNumTransforms;
		AnimOut.mNumFrames = Anim.mNumFrames;
		AnimOut.transformTypeOffset = Anim.transformTypeOffset;
		AnimOut.frameTimesOffset = Anim.frameTimesOffset;
		AnimOut.xformFramesOffset = Anim.xformFramesOffset;
		AnimOut.alphaFramesOffset = Anim.alphaFramesOffset;
		AnimOut.colorFramesOffset = Anim.colorFramesOffset;
		AnimOut.soundFramesOffset = Anim.soundFramesOffset;
		AnimOut.stringTableOffset = Anim.stringTableOffset;
		stream.seekp(offset, ios::beg);
		stream.write((char*)&AnimOut, sizeof(AnimIO));
		//write framTimes
		stream.seekp(Anim.animOffset + Anim.frameTimesOffset, ios::beg);
		for (int i = 0; i < Anim.mNumFrames; i++)
			stream.write((char*)(Anim.mFrameTimes + i), sizeof(float));

		keyframeIO keyOut;
		stream.seekp(Anim.animOffset + Anim.transformTypeOffset, ios::beg);
		for (int i = 0; i < Anim.mNumTransforms; i++)
			stream.write((char*)(Anim.mTransformTypes + i), sizeof(TransformType));

		//write Transforms
		if (Anim.mHasTransform) {
			stream.seekp(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumTransforms; i++)
				stream.write((char*)(Anim.TransFrameOffset + i), sizeof(int));

			//write xformFramesOffset
			stream.seekp(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++) {
				stream.seekp(Anim.animOffset + Anim.TransFrameOffset[i], ios::beg);
				for (int j = 0; j < Anim.mNumFrames; j++) {
					stream.write((char*)(Anim.TransSingleFrameOffset[i] + j), sizeof(int));
				}
				for (int j = 0; j < Anim.mNumFrames; j++)
					if (Anim.mXformFrames[i][j].isValid)
					{
						keyOut.alpha = Anim.mXformFrames[i][j].alpha;
						keyOut.angle = Anim.mXformFrames[i][j].angle;
						keyOut.color = Anim.mXformFrames[i][j].color;
						keyOut.interpolation = Anim.mXformFrames[i][j].interpolation;
						keyOut.nextFrameIndex = Anim.mXformFrames[i][j].nextFrameIndex;
						keyOut.soundStrIdx = Anim.mXformFrames[i][j].soundStrIdx;
						keyOut.x = Anim.mXformFrames[i][j].x;
						keyOut.y = Anim.mXformFrames[i][j].y;
						stream.seekp(Anim.TransSingleFrameOffset[i][j] + Anim.animOffset, ios::beg);
						stream.write((char*)&keyOut, sizeof(keyframeIO));
					}
				for (int j = 0; j < Anim.mNumFrames; j++)
					if (Anim.mXformFrames[i][j].soundStrIdx)
						writeStr(stream, Anim.mXformFrames[i][j].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mXformFrames[i][j].soundString);
			}
		}


		//writeAlpha
		if (Anim.mHasAlpha) {
			stream.seekp(Anim.animOffset + Anim.alphaFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.write((char*)(Anim.AlphaOffset + i), sizeof(int));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].isValid) {
					keyOut.alpha = Anim.mAlphaFrames[i].alpha;
					keyOut.angle = Anim.mAlphaFrames[i].angle;
					keyOut.color = Anim.mAlphaFrames[i].color;
					keyOut.interpolation = Anim.mAlphaFrames[i].interpolation;
					keyOut.nextFrameIndex = Anim.mAlphaFrames[i].nextFrameIndex;
					keyOut.soundStrIdx = Anim.mAlphaFrames[i].soundStrIdx;
					keyOut.x = Anim.mAlphaFrames[i].x;
					keyOut.y = Anim.mAlphaFrames[i].y;
					stream.seekp(Anim.animOffset + Anim.AlphaOffset[i], ios::beg);
					stream.write((char*)&keyOut, sizeof(keyframeIO));
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].soundStrIdx)
					writeStr(stream, Anim.mAlphaFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mAlphaFrames[i].soundString);

		}

		//writeColor
		if (Anim.mHasColor) {
			stream.seekp(Anim.animOffset + Anim.colorFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.write((char*)(Anim.ColorOffset + i), sizeof(int));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].isValid) {
					keyOut.alpha = Anim.mColorFrames[i].alpha;
					keyOut.angle = Anim.mColorFrames[i].angle;
					keyOut.color = Anim.mColorFrames[i].color;
					keyOut.interpolation = Anim.mColorFrames[i].interpolation;
					keyOut.nextFrameIndex = Anim.mColorFrames[i].nextFrameIndex;
					keyOut.soundStrIdx = Anim.mColorFrames[i].soundStrIdx;
					keyOut.x = Anim.mColorFrames[i].x;
					keyOut.y = Anim.mColorFrames[i].y;
					stream.seekp(Anim.animOffset + Anim.ColorOffset[i], ios::beg);
					stream.write((char*)&keyOut, sizeof(keyframeIO));
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].soundStrIdx)
					writeStr(stream, Anim.mColorFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mColorFrames[i].soundString);

		}

		//writeSound
		if (Anim.mHasSound) {
			stream.seekp(Anim.animOffset + Anim.soundFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.write((char*)(Anim.SoundOffset + i), sizeof(int));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].isValid) {
					keyOut.alpha = Anim.mSoundFrames[i].alpha;
					keyOut.angle = Anim.mSoundFrames[i].angle;
					keyOut.color = Anim.mSoundFrames[i].color;
					keyOut.interpolation = Anim.mSoundFrames[i].interpolation;
					keyOut.nextFrameIndex = Anim.mSoundFrames[i].nextFrameIndex;
					keyOut.soundStrIdx = Anim.mSoundFrames[i].soundStrIdx;
					keyOut.x = Anim.mSoundFrames[i].x;
					keyOut.y = Anim.mSoundFrames[i].y;
					stream.seekp(Anim.animOffset + Anim.SoundOffset[i], ios::beg);
					stream.write((char*)&keyOut, sizeof(keyframeIO));
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].soundStrIdx)
					writeStr(stream, Anim.mSoundFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mSoundFrames[i].soundString);

		}

	}

	void output(ofstream& stream, int offset, BinMovie& Movie) {
		stream.seekp(offset, ios::beg);
		MovieIO MovieOut;

		MovieOut.length = Movie.length;
		MovieOut.numActors = Movie.numActors;
		AnimIO* AnimOut = new AnimIO[Movie.numActors];
		ActorIO* ActorOut = new ActorIO[Movie.numActors];
		MovieOut.actorsOffset = Movie.actorsOffset;
		MovieOut.animsOffset = Movie.animsOffset;
		MovieOut.stringsOffset = Movie.stringsOffset;

		stream.write((char*)&MovieOut, sizeof(MovieIO));

		//write Actors
		stream.seekp(offset + Movie.actorsOffset, ios::beg);
		for (int i = 0; i < Movie.numActors; i++) {
			ActorOut[i].mType = Movie.pActors[i].mType;
			ActorOut[i].mImageStrIdx = Movie.pActors[i].mImageStrIdx;
			ActorOut[i].mLabelTextStrIdx = Movie.pActors[i].mLabelTextStrIdx;
			ActorOut[i].mFontStrIdx = Movie.pActors[i].mFontStrIdx;
			ActorOut[i].mLabelMaxWidth = Movie.pActors[i].mLabelMaxWidth;
			ActorOut[i].mLabelWrapWidth = Movie.pActors[i].mLabelWrapWidth;
			ActorOut[i].mLabelJustification = Movie.pActors[i].mLabelJustification;
			ActorOut[i].mDepth = Movie.pActors[i].mDepth;
			stream.write((char*)(ActorOut + i), sizeof(ActorIO));
		}
		for (int i = 0; i < MovieOut.numActors; i++) {
			writeStr(stream, Movie.stringsOffset + Movie.pActors[i].mImageStrIdx, Movie.pActors[i].ImageStr);
			writeStr(stream, Movie.stringsOffset + Movie.pActors[i].mLabelTextStrIdx, Movie.pActors[i].LabelTextStr);
			writeStr(stream, Movie.stringsOffset + Movie.pActors[i].mFontStrIdx, Movie.pActors[i].FontStr);
		}

		//read Anim offset
		stream.seekp(offset + MovieOut.animsOffset, ios::beg);
		for (int i = 0; i < MovieOut.numActors; i++)
			stream.write((char*)&(Movie.pAnims[i].animOffset), sizeof(int));
		for (int i = 0; i < MovieOut.numActors; i++) {
			output(stream, Movie.pAnims[i].animOffset, Movie.pAnims[i]);
		}
	}

	int alloc(int& stringOffset, vector<string>& strList, vector<int>& pos, string str) {
		int currentLoc = stringOffset;
		for (int i = 0; i < strList.size(); i++)
			if (str == strList[i])
				return pos[i];
		stringOffset += str.length() + 1;
		strList.push_back(str);
		pos.push_back(currentLoc);
		return currentLoc;
	}

	int deploy(int& GlobOffset, int stringOffset) {
		int currentPos = GlobOffset;
		GlobOffset += stringOffset + 3;
		return currentPos;
	}

	void alloc(int& GlobOffset, BinImageAnimation& Anim) {
		vector<string>strList;
		vector<int>pos;
		int strOffset = 0;
		int selfOffset = GlobOffset;

		//stream.seekg(offset, ios::beg);
		Anim.animOffset = GlobOffset;														//offset=0

		//stream.read((char*)&AnimIn, sizeof(AnimIO));
		GlobOffset += sizeof(AnimIO);

		//stream.seekg(Anim.animOffset + Anim.transformTypeOffset, ios::beg);
		Anim.transformTypeOffset = GlobOffset - Anim.animOffset;							//offset=52

		//for (int i = 0; i < Anim.mNumTransforms; i++)
			//stream.read((char*)(Anim.mTransformTypes  + i), sizeof(TransformType));
		GlobOffset += sizeof(TransformType) * Anim.mNumTransforms;

		//stream.seekg(Anim.animOffset + Anim.frameTimesOffset, ios::beg);
		Anim.frameTimesOffset = GlobOffset - Anim.animOffset;								//offset=64

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.mFrameTimes + i), sizeof(float));
		GlobOffset += sizeof(float) * Anim.mNumFrames;

		//stream.seekg(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
		Anim.xformFramesOffset = GlobOffset - Anim.animOffset;								//offset=72

		//for (int i = 0; i < Anim.mNumTransforms; i++)
			//stream.read((char*)(Anim.TransFrameOffset + i), sizeof(int));
		GlobOffset += sizeof(int) * Anim.mNumTransforms;									//offset=84

		//getTransform
		if (Anim.mHasTransform) {

			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++) {

				//stream.seekg(Anim.animOffset + Anim.TransFrameOffset[i], ios::beg);
				Anim.TransFrameOffset[i] = GlobOffset - Anim.animOffset;

				//for (int j = 0; j < Anim.mNumFrames; j++)
					//stream.read((char*)(Anim.TransSingleFrameOffset[i] + j), sizeof(int));
				GlobOffset += sizeof(int) * Anim.mNumFrames;
			}
		}

		//stream.seekg(Anim.animOffset + Anim.alphaFramesOffset, ios::beg);
		Anim.alphaFramesOffset = GlobOffset - Anim.animOffset;

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.AlphaOffset + i), sizeof(int));
		GlobOffset += sizeof(int) * Anim.mNumFrames;										//offset=108

		//stream.seekg(Anim.animOffset + Anim.colorFramesOffset, ios::beg);
		Anim.colorFramesOffset = GlobOffset - Anim.animOffset;

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.ColorOffset + i), sizeof(int));
		GlobOffset += sizeof(int) * Anim.mNumFrames;										//offset=116

		//stream.seekg(Anim.animOffset + Anim.soundFramesOffset, ios::beg);
		Anim.soundFramesOffset = GlobOffset - Anim.animOffset;

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.SoundOffset + i), sizeof(int));
		GlobOffset += sizeof(int) * Anim.mNumFrames;										//offset=124

		//string assign begin
		if (Anim.mHasTransform) {
			for (int i = 0; i < Anim.mNumTransforms; i++)
				for (int j = 0; j < Anim.mNumFrames; j++)
					//if (Anim.mXformFrames[i][j].soundStrIdx)
						//Anim.mXformFrames[i][j].soundString = readStr(stream, Anim.mXformFrames[i][j].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
					if (Anim.mXformFrames[i][j].isValid)
						Anim.mXformFrames[i][j].soundStrIdx = alloc(strOffset, strList, pos, Anim.mXformFrames[i][j].soundString);
		}

		if (Anim.mHasAlpha) {
			for (int i = 0; i < Anim.mNumFrames; i++)
				//if (Anim.mAlphaFrames[i].soundStrIdx)
					//Anim.mAlphaFrames[i].soundString = readStr(stream, Anim.mAlphaFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				if (Anim.mAlphaFrames[i].isValid)
					Anim.mAlphaFrames[i].soundStrIdx = alloc(strOffset, strList, pos, Anim.mAlphaFrames[i].soundString);
		}

		if (Anim.mHasColor) {
			for (int i = 0; i < Anim.mNumFrames; i++)
				//if (Anim.mColorFrames[i].soundStrIdx)
					//Anim.mColorFrames[i].soundString = readStr(stream, Anim.mColorFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				if (Anim.mAlphaFrames[i].isValid)
					Anim.mAlphaFrames[i].soundStrIdx = alloc(strOffset, strList, pos, Anim.mColorFrames[i].soundString);
		}

		if (Anim.mHasSound) {
			for (int i = 0; i < Anim.mNumFrames; i++)
				//if (Anim.mSoundFrames[i].soundStrIdx)
					//Anim.mSoundFrames[i].soundString = readStr(stream, Anim.mSoundFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				if (Anim.mSoundFrames[i].isValid)
					Anim.mSoundFrames[i].soundStrIdx = alloc(strOffset, strList, pos, Anim.mSoundFrames[i].soundString);
		}

		Anim.stringTableOffset = deploy(GlobOffset, strOffset) - Anim.animOffset;
		//string assign end

		if (Anim.mHasTransform) {
			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++)
				for (int j = 0; j < Anim.mNumFrames; j++) {
					//stream.seekg(Anim.TransSingleFrameOffset[i][j] + Anim.animOffset, ios::beg);
					if (Anim.mXformFrames[i][j].isValid) {
						Anim.TransSingleFrameOffset[i][j] = GlobOffset - Anim.animOffset;

						//stream.read((char*)&keyIn, sizeof(keyframeIO));
						GlobOffset += sizeof(keyframeIO);
					}
					else {
						Anim.TransSingleFrameOffset[i][j] = 0;
					}
				}
		}

		//getAlpha
		if (Anim.mHasAlpha) {

			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].isValid)
				{
					//stream.seekg(Anim.animOffset + Anim.AlphaOffset[i], ios::beg);
					Anim.AlphaOffset[i] = GlobOffset - Anim.animOffset;

					//stream.read((char*)&keyIn, sizeof(keyframeIO));
					GlobOffset += sizeof(keyframeIO);
				}
				else {
					Anim.AlphaOffset[i] = 0;
				}
		}

		//getColor
		if (Anim.mHasColor) {

			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].isValid)
				{
					//stream.seekg(Anim.animOffset + Anim.ColorOffset[i], ios::beg);
					Anim.ColorOffset[i] = GlobOffset - Anim.animOffset;

					//stream.read((char*)&keyIn, sizeof(keyframeIO));
					GlobOffset += sizeof(keyframeIO);
				}
				else {
					Anim.ColorOffset[i] = 0;
				}
		}
		//getSound
		if (Anim.mHasSound) {
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].isValid)
				{
					//stream.seekg(Anim.animOffset + Anim.SoundOffset[i], ios::beg);
					Anim.SoundOffset[i] = GlobOffset - Anim.animOffset;

					//stream.read((char*)&keyIn, sizeof(keyframeIO));
					GlobOffset += sizeof(keyframeIO);

				}
				else {
					Anim.SoundOffset[i] = 0;
				}
		}
	}

	void alloc(int& GlobOffset, BinMovie& Movie) {
		vector<string>strList;
		vector<int>pos;
		int strOffset = 0;
		int offset;

		//stream.seekg(offset, ios::beg);
		offset = GlobOffset;

		//stream.read((char*)&MovieIn, sizeof(MovieIO));
		GlobOffset += sizeof(MovieIO);

		//read Actors
		//stream.seekg(offset + MovieIn.actorsOffset, ios::beg);
		Movie.actorsOffset = GlobOffset - offset;

		//for (int i = 0; i < MovieIn.numActors; i++)
			//stream.read((char*)(ActorIn + i), sizeof(ActorIO));
		GlobOffset += sizeof(ActorIO) * Movie.numActors;

		//read Anim offset
		//stream.seekg(offset + MovieIn.animsOffset, ios::beg);
		Movie.animsOffset = GlobOffset - offset;

		for (int i = 0; i < Movie.numActors; i++) {
			//Movie.pActors[i].ImageStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mImageStrIdx);
			Movie.pActors[i].mImageStrIdx = alloc(strOffset, strList, pos, Movie.pActors[i].ImageStr);

			//Movie.pActors[i].LabelTextStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mLabelTextStrIdx);
			Movie.pActors[i].mLabelTextStrIdx = alloc(strOffset, strList, pos, Movie.pActors[i].LabelTextStr);

			//Movie.pActors[i].FontStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mFontStrIdx);
			Movie.pActors[i].mFontStrIdx = alloc(strOffset, strList, pos, Movie.pActors[i].FontStr);
		}

		//for (int i = 0; i < MovieIn.numActors; i++)
			//stream.read((char*)(AnimOffset + i), sizeof(int));
		GlobOffset += sizeof(int) * Movie.numActors;

		Movie.stringsOffset = deploy(GlobOffset, strOffset) - offset;

		for (int i = 0; i < Movie.numActors; i++) {
			//ReadAnim(stream, AnimOffset[i], Movie.pAnims[i]);
			Movie.pAnims[i].animOffset = GlobOffset;
			alloc(GlobOffset, Movie.pAnims[i]);
		}


	}

}

namespace newFormat {

	enum TransformType {
		XFORM_SCALE = 0,
		XFORM_ROTATE = 1,
		XFORM_TRANSLATE = 2
	};

	enum InterpolationType {
		INTERPOLATION_NONE = 0,
		INTERPOLATION_LINEAR = 1
	};

	enum ActorType
	{
		eActorType_Image = 0,
		eActorType_Text = 1
	};

	enum AlignmentH
	{
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2
	};

	enum AlignmentV
	{
		ALIGN_TOP = 0,
		ALIGN_MIDDLE = 1,
		ALIGN_BOTTOM = 2
	};

	struct keyframe
	{
		bool isValid = false;
		float x = 0.f;
		float y = 0.f;
		float angle = 0.f;
		int alpha = 0;
		int color = 0;
		int nextFrameIndex = 0;						//maybe Long Long?
		int soundStrIdx = 0;						//seem to be int?
		InterpolationType interpolation = InterpolationType::INTERPOLATION_LINEAR;
		string soundString = "";
	};

	struct keyframeIO
	{
		float x = 0.f;
		float y = 0.f;
		float angle = 0.f;
		int alpha = 0;
		int color = 0;
		int nextFrameIndex = 0;						//maybe Long Long?
		int soundStrIdx = 0;						//seem to be int?
		InterpolationType interpolation = InterpolationType::INTERPOLATION_LINEAR;
	};

	struct BinImageAnimation
	{
		long long animOffset = 0;

		int mHasColor = 0;
		int mHasAlpha = 0;
		int mHasSound = 0;
		int mHasTransform = 0;
		int mNumTransforms = 0;
		int mNumFrames = 0;

		long long transformTypeOffset = 0;
		long long frameTimesOffset = 0;
		long long xformFramesOffset = 0;
		long long alphaFramesOffset = 0;
		long long colorFramesOffset = 0;
		long long soundFramesOffset = 0;
		long long stringTableOffset = 0;

		TransformType* mTransformTypes;
		long long TransFrameOffset[3] = { 0 };
		long long* AlphaOffset;
		long long* ColorOffset;
		long long* SoundOffset;
		long long* TransSingleFrameOffset[3];
		float* mFrameTimes;
		keyframe* mXformFrames[3];
		keyframe* mAlphaFrames;
		keyframe* mColorFrames;
		keyframe* mSoundFrames;
	};

	struct AnimIO {
		char AnimIdentifier[8] = { 'B','I','N','U','N','I','A','N' };
		int mHasColor = 0;
		int mHasAlpha = 0;
		int mHasSound = 0;
		int mHasTransform = 0;
		int mNumTransforms = 0;
		int mNumFrames = 0;

		long long transformTypeOffset = 0;
		long long frameTimesOffset = 0;
		long long xformFramesOffset = 0;
		long long alphaFramesOffset = 0;
		long long colorFramesOffset = 0;
		long long soundFramesOffset = 0;
		long long stringTableOffset = 0;
	};

	struct BinActor
	{
		ActorType mType = ActorType::eActorType_Image;
		int mImageStrIdx = 0;
		string ImageStr = "";
		int mLabelTextStrIdx = 0;
		string LabelTextStr = "";
		int mFontStrIdx = 0;
		string FontStr = "";
		float mLabelMaxWidth = 0.f;
		float mLabelWrapWidth = 0.f;
		AlignmentH mLabelJustification = AlignmentH::ALIGN_CENTER;
		float mDepth = 0.f;
	};

	struct ActorIO
	{
		ActorType mType = ActorType::eActorType_Image;
		int mImageStrIdx = 0;
		int mLabelTextStrIdx = 0;
		int mFontStrIdx = 0;
		float mLabelMaxWidth = 0.f;
		float mLabelWrapWidth = 0.f;
		AlignmentH mLabelJustification = AlignmentH::ALIGN_CENTER;
		float mDepth = 0.f;
	};

	struct MovieIO {
		char AnimIdentifier[8] = { 'B','I','N','U','N','I','M','O' };
		float length = 0.f;
		int numActors = 0;
		long long actorsOffset = 0;
		long long animsOffset = 0;
		long long stringsOffset = 0;
	};

	struct BinMovie
	{
		float length = 0.f;
		int numActors = 0;
		BinActor* pActors;
		BinImageAnimation* pAnims;

		//inputed
		long long actorsOffset = 0;
		long long animsOffset = 0;
		long long stringsOffset = 0;
	};

	string readStr(ifstream& stream, long long offset) {
		stream.seekg(offset, ios::beg);
		char chr;
		string str = "";
		stream.read(&chr, sizeof(char));
		while ((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') || chr == '_' || chr == '$') {
			str += chr;
			stream.read(&chr, sizeof(char));
		}
		return str;
	}

	void writeStr(ofstream& stream, long long offset, string str) {
		stream.seekp(offset, ios::beg);
		stream.write(str.c_str(), sizeof(char) * str.length());
	}

	void ReadAnim(ifstream& stream, long long offset, BinImageAnimation& Anim) {
		AnimIO AnimIn;
		stream.seekg(offset, ios::beg);
		stream.read((char*)&AnimIn, sizeof(AnimIO));
		Anim.mHasColor = AnimIn.mHasColor;
		Anim.mHasAlpha = AnimIn.mHasAlpha;
		Anim.mHasSound = AnimIn.mHasSound;
		Anim.mHasTransform = AnimIn.mHasTransform;
		Anim.mNumTransforms = AnimIn.mNumTransforms;
		Anim.mNumFrames = AnimIn.mNumFrames;
		Anim.transformTypeOffset = AnimIn.transformTypeOffset;
		Anim.frameTimesOffset = AnimIn.frameTimesOffset;
		Anim.xformFramesOffset = AnimIn.xformFramesOffset;
		Anim.alphaFramesOffset = AnimIn.alphaFramesOffset;
		Anim.colorFramesOffset = AnimIn.colorFramesOffset;
		Anim.soundFramesOffset = AnimIn.soundFramesOffset;
		Anim.stringTableOffset = AnimIn.stringTableOffset;
		Anim.animOffset = offset;

		//get framTimes
		stream.seekg(Anim.animOffset + Anim.frameTimesOffset, ios::beg);
		Anim.mFrameTimes = new float[Anim.mNumFrames];
		for (int i = 0; i < Anim.mNumFrames; i++)
			stream.read((char*)(Anim.mFrameTimes + i), sizeof(float));

		keyframeIO keyIn;

		//get Transform Types
		stream.seekg(Anim.animOffset + Anim.transformTypeOffset, ios::beg);
		Anim.mTransformTypes = new TransformType[Anim.mNumTransforms];
		for (int i = 0; i < Anim.mNumTransforms; i++)
			stream.read((char*)(Anim.mTransformTypes + i), sizeof(TransformType));

		//get Transforms
		if (Anim.mHasTransform) {
			//get xformFramesOffset
			stream.seekg(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumTransforms; i++)
				stream.read((char*)(Anim.TransFrameOffset + i), sizeof(long long));

			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++) {
				Anim.mXformFrames[i] = new keyframe[Anim.mNumFrames];
				Anim.TransSingleFrameOffset[i] = new long long[Anim.mNumFrames];
				stream.seekg(Anim.animOffset + Anim.TransFrameOffset[i], ios::beg);
				for (int j = 0; j < Anim.mNumFrames; j++) {
					stream.read((char*)(Anim.TransSingleFrameOffset[i] + j), sizeof(long long));
				}
				for (int j = 0; j < Anim.mNumFrames; j++) {
					if (Anim.TransSingleFrameOffset[i][j] == 0) {
						Anim.mXformFrames[i][j].isValid = false;
					}
					else {
						Anim.mXformFrames[i][j].isValid = true;
						stream.seekg(Anim.TransSingleFrameOffset[i][j] + Anim.animOffset, ios::beg);
						stream.read((char*)&keyIn, sizeof(keyframeIO));
						Anim.mXformFrames[i][j].alpha = keyIn.alpha;
						Anim.mXformFrames[i][j].angle = keyIn.angle;
						Anim.mXformFrames[i][j].color = keyIn.color;
						Anim.mXformFrames[i][j].interpolation = keyIn.interpolation;
						Anim.mXformFrames[i][j].nextFrameIndex = keyIn.nextFrameIndex;
						Anim.mXformFrames[i][j].soundStrIdx = keyIn.soundStrIdx;
						Anim.mXformFrames[i][j].x = keyIn.x;
						Anim.mXformFrames[i][j].y = keyIn.y;
					}
				}
				for (int j = 0; j < Anim.mNumFrames; j++)
					if (Anim.mXformFrames[i][j].soundStrIdx)
						Anim.mXformFrames[i][j].soundString = readStr(stream, Anim.mXformFrames[i][j].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
					else
						Anim.mXformFrames[i][j].soundString = "";
			}
		}


		//getAlpha
		if (Anim.mHasAlpha) {
			stream.seekg(Anim.animOffset + Anim.alphaFramesOffset, ios::beg);
			Anim.mAlphaFrames = new keyframe[Anim.mNumFrames];
			Anim.AlphaOffset = new long long[Anim.mNumFrames];
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.read((char*)(Anim.AlphaOffset + i), sizeof(long long));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.AlphaOffset[i] == 0) {
					Anim.mAlphaFrames[i].isValid = false;
				}
				else {
					Anim.mAlphaFrames[i].isValid = true;
					stream.seekg(Anim.animOffset + Anim.AlphaOffset[i], ios::beg);
					stream.read((char*)&keyIn, sizeof(keyframeIO));
					Anim.mAlphaFrames[i].alpha = keyIn.alpha;
					Anim.mAlphaFrames[i].angle = keyIn.angle;
					Anim.mAlphaFrames[i].color = keyIn.color;
					Anim.mAlphaFrames[i].interpolation = keyIn.interpolation;
					Anim.mAlphaFrames[i].nextFrameIndex = keyIn.nextFrameIndex;
					Anim.mAlphaFrames[i].soundStrIdx = keyIn.soundStrIdx;
					Anim.mAlphaFrames[i].x = keyIn.x;
					Anim.mAlphaFrames[i].y = keyIn.y;
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].soundStrIdx)
					Anim.mAlphaFrames[i].soundString = readStr(stream, Anim.mAlphaFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				else
					Anim.mAlphaFrames[i].soundString = "";

		}

		//getColor
		if (Anim.mHasColor) {
			stream.seekg(Anim.animOffset + Anim.colorFramesOffset, ios::beg);
			Anim.mColorFrames = new keyframe[Anim.mNumFrames];
			Anim.ColorOffset = new long long[Anim.mNumFrames];
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.read((char*)(Anim.ColorOffset + i), sizeof(long long));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.ColorOffset[i] == 0) {
					Anim.mColorFrames[i].isValid = false;
				}
				else {
					Anim.mColorFrames[i].isValid = true;
					stream.seekg(Anim.animOffset + Anim.ColorOffset[i], ios::beg);
					stream.read((char*)&keyIn, sizeof(keyframeIO));
					Anim.mColorFrames[i].alpha = keyIn.alpha;
					Anim.mColorFrames[i].angle = keyIn.angle;
					Anim.mColorFrames[i].color = keyIn.color;
					Anim.mColorFrames[i].interpolation = keyIn.interpolation;
					Anim.mColorFrames[i].nextFrameIndex = keyIn.nextFrameIndex;
					Anim.mColorFrames[i].soundStrIdx = keyIn.soundStrIdx;
					Anim.mColorFrames[i].x = keyIn.x;
					Anim.mColorFrames[i].y = keyIn.y;
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].soundStrIdx)
					Anim.mColorFrames[i].soundString = readStr(stream, Anim.mColorFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				else
					Anim.mColorFrames[i].soundString = "";

		}

		//getSound
		if (Anim.mHasSound) {
			stream.seekg(Anim.animOffset + Anim.soundFramesOffset, ios::beg);
			Anim.mSoundFrames = new keyframe[Anim.mNumFrames];
			Anim.SoundOffset = new long long[Anim.mNumFrames];
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.read((char*)(Anim.SoundOffset + i), sizeof(long long));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.SoundOffset[i] == 0) {
					Anim.mSoundFrames[i].isValid = false;
				}
				else {
					Anim.mSoundFrames[i].isValid = true;
					stream.seekg(Anim.animOffset + Anim.SoundOffset[i], ios::beg);
					stream.read((char*)&keyIn, sizeof(keyframeIO));
					Anim.mSoundFrames[i].alpha = keyIn.alpha;
					Anim.mSoundFrames[i].angle = keyIn.angle;
					Anim.mSoundFrames[i].color = keyIn.color;
					Anim.mSoundFrames[i].interpolation = keyIn.interpolation;
					Anim.mSoundFrames[i].nextFrameIndex = keyIn.nextFrameIndex;
					Anim.mSoundFrames[i].soundStrIdx = keyIn.soundStrIdx;
					Anim.mSoundFrames[i].x = keyIn.x;
					Anim.mSoundFrames[i].y = keyIn.y;
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].soundStrIdx)
					Anim.mSoundFrames[i].soundString = readStr(stream, Anim.mSoundFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				else
					Anim.mSoundFrames[i].soundString = "";
		}

	}

	void ReadMovie(ifstream& stream, long long offset, BinMovie& Movie) {
		stream.seekg(offset, ios::beg);
		MovieIO MovieIn;

		stream.read((char*)&MovieIn, sizeof(MovieIO));

		Movie.length = MovieIn.length;
		Movie.numActors = MovieIn.numActors;
		Movie.pActors = new BinActor[MovieIn.numActors];
		Movie.pAnims = new BinImageAnimation[MovieIn.numActors];
		AnimIO* AnimIn = new AnimIO[MovieIn.numActors];
		ActorIO* ActorIn = new ActorIO[MovieIn.numActors];
		Movie.actorsOffset = MovieIn.actorsOffset;
		Movie.animsOffset = MovieIn.animsOffset;
		Movie.stringsOffset = MovieIn.stringsOffset;

		//read Actors
		stream.seekg(offset + MovieIn.actorsOffset, ios::beg);
		for (int i = 0; i < MovieIn.numActors; i++) {
			stream.read((char*)(ActorIn + i), sizeof(ActorIO));
			Movie.pActors[i].mType = ActorIn[i].mType;
			Movie.pActors[i].mImageStrIdx = ActorIn[i].mImageStrIdx;
			Movie.pActors[i].mLabelTextStrIdx = ActorIn[i].mLabelTextStrIdx;
			Movie.pActors[i].mFontStrIdx = ActorIn[i].mFontStrIdx;
			Movie.pActors[i].mLabelMaxWidth = ActorIn[i].mLabelMaxWidth;
			Movie.pActors[i].mLabelWrapWidth = ActorIn[i].mLabelWrapWidth;
			Movie.pActors[i].mLabelJustification = ActorIn[i].mLabelJustification;
			Movie.pActors[i].mDepth = ActorIn[i].mDepth;
		}
		for (int i = 0; i < MovieIn.numActors; i++) {
			Movie.pActors[i].ImageStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mImageStrIdx);
			Movie.pActors[i].LabelTextStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mLabelTextStrIdx);
			Movie.pActors[i].FontStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mFontStrIdx);
		}

		//read Anim offset
		stream.seekg(offset + MovieIn.animsOffset, ios::beg);
		long long* AnimOffset = new long long[MovieIn.numActors];
		for (int i = 0; i < MovieIn.numActors; i++)
			stream.read((char*)(AnimOffset + i), sizeof(long long));
		for (int i = 0; i < MovieIn.numActors; i++) {
			ReadAnim(stream, AnimOffset[i], Movie.pAnims[i]);
		}
	}

	string getType(BinActor& actor) {
		switch (actor.mType) {
		case 0:
			return string("IMAGE");
		case 1:
			return string("TEXT");
		default:
			return string("INVALID");
		}
	}

	string getTransformType(TransformType& Transform) {
		switch (Transform) {
		case 0:
			return string("SCALE");
		case 1:
			return string("ROTATE");
		case 2:
			return string("TRANSLATE");
		default:
			return string("INVALID");
		}
	}

	string getInterpolationType(InterpolationType& Int) {
		switch (Int) {
		case 0:
			return string("NONE");
		case 1:
			return string("LINEAR");
		default:
			return to_string(Int);
		}
	}

	string B2S(bool in) {
		return in ? "true" : "false";
	}

	template<typename _T>
	string to_string(_T x) {
		std::ostringstream ss;
		ss << x;
		return ss.str();
	}

	void print(BinActor& actor) {
		cout << "actor = BinActor{actorType=" << getType(actor) << ", imageStr='" << actor.ImageStr << "', labelStr='" << actor.LabelTextStr << "', fontStr='" << actor.FontStr << "', labelMaxWidth=" << actor.mLabelMaxWidth << ", labelWrapWidth=" << actor.mLabelWrapWidth << ", labelJustification=" << actor.mLabelJustification << ", depth=" << actor.mDepth << "}" << endl;
	}

	void print(BinImageAnimation& anim) {
		printf("binImageAnimOffset = %lld\n", anim.animOffset);
		cout << "hasColor = " << B2S(anim.mHasColor) << endl;
		cout << "hasAlpha = " << B2S(anim.mHasAlpha) << endl;
		cout << "hasSound = " << B2S(anim.mHasSound) << endl;
		cout << "hasTransform = " << B2S(anim.mHasTransform) << endl;
		cout << "numTransforms = " << anim.mNumTransforms << endl;
		cout << "numFrames = " << anim.mNumFrames << endl;
		cout << "transformTypeOffset = " << anim.animOffset + anim.transformTypeOffset << endl;
		cout << "frameTimesOffset = " << anim.animOffset + anim.frameTimesOffset << endl;
		cout << "xformFramesOffset = " << anim.animOffset + anim.xformFramesOffset << endl;
		cout << "alphaFramesOffset = " << anim.animOffset + anim.alphaFramesOffset << endl;
		cout << "colorFramesOffset = " << anim.animOffset + anim.colorFramesOffset << endl;
		cout << "soundFramesOffset = " << anim.animOffset + anim.soundFramesOffset << endl;
		cout << "stringTableOffset = " << anim.animOffset + anim.stringTableOffset << endl;

		for (int i = 0; i < anim.mNumFrames; i++)
			cout << "frameTimes[" << i << "] = " << anim.mFrameTimes[i] << endl;

		if (anim.mHasTransform) {
			for (int i = 0; i < anim.mNumTransforms; i++)
				cout << "transformTypes[" << i << "] = " << getTransformType(anim.mTransformTypes[i]) << endl;
			for (int i = 0; i < anim.mNumTransforms; i++) {
				cout << "frameOffset = " << anim.TransFrameOffset[i] + anim.animOffset << endl;
				for (int j = 0; j < anim.mNumFrames; j++)
					if (anim.mXformFrames[i][j].isValid)
						cout << "( " << anim.TransSingleFrameOffset[i][j] << " )transformFrames[t=" << anim.mTransformTypes[i] << "," << getTransformType(anim.mTransformTypes[i]) << "][frame=" << j << "] = KeyFrame {x=" << anim.mXformFrames[i][j].x << ", y=" << anim.mXformFrames[i][j].y << ", angle=" << anim.mXformFrames[i][j].angle << ", alpha=" << anim.mXformFrames[i][j].alpha << ", color=" << anim.mXformFrames[i][j].color << ", nextFrameIndex=" << anim.mXformFrames[i][j].nextFrameIndex << ", soundString=" << soundStr(anim.mXformFrames[i][j]) << ", interpolationType=" << getInterpolationType(anim.mXformFrames[i][j].interpolation) << "}" << endl;
			}
		}
		if (anim.mHasAlpha)
			for (int i = 0; i < anim.mNumFrames; i++)
				if (anim.mAlphaFrames[i].isValid)
					cout << "alphaFrames[" << i << "] = KeyFrame {x=" << anim.mAlphaFrames[i].x << ", y=" << anim.mAlphaFrames[i].y << ", angle=" << anim.mAlphaFrames[i].angle << ", alpha=" << anim.mAlphaFrames[i].alpha << ", color=" << anim.mAlphaFrames[i].color << ", nextFrameIndex=" << anim.mAlphaFrames[i].nextFrameIndex << ", soundString=" << soundStr(anim.mAlphaFrames[i]) << ", interpolationType=" << getInterpolationType(anim.mAlphaFrames[i].interpolation) << "}" << endl;

		if (anim.mHasColor)
			for (int i = 0; i < anim.mNumFrames; i++)
				if (anim.mColorFrames[i].isValid)
					cout << "colorFrames[" << i << "] = KeyFrame {x=" << anim.mColorFrames[i].x << ", y=" << anim.mColorFrames[i].y << ", angle=" << anim.mColorFrames[i].angle << ", alpha=" << anim.mColorFrames[i].alpha << ", color=" << anim.mColorFrames[i].color << ", nextFrameIndex=" << anim.mColorFrames[i].nextFrameIndex << ", soundString=" << soundStr(anim.mColorFrames[i]) << ", interpolationType=" << getInterpolationType(anim.mColorFrames[i].interpolation) << "}" << endl;

		if (anim.mHasSound)
			for (int i = 0; i < anim.mNumFrames; i++)
				if (anim.mSoundFrames[i].isValid)
					cout << "soundFrames[" << i << "] = KeyFrame {x=" << anim.mSoundFrames[i].x << ", y=" << anim.mSoundFrames[i].y << ", angle=" << anim.mSoundFrames[i].angle << ", alpha=" << anim.mSoundFrames[i].alpha << ", color=" << anim.mSoundFrames[i].color << ", nextFrameIndex=" << anim.mSoundFrames[i].nextFrameIndex << ", soundString=" << soundStr(anim.mSoundFrames[i]) << ", interpolationType=" << getInterpolationType(anim.mSoundFrames[i].interpolation) << "}" << endl;

	}

	void print(BinMovie& movie) {
		printf("length = %f\n", movie.length);
		printf("numActors = %d\n", movie.numActors);
		printf("actorsOffset = %lld\n", movie.actorsOffset);
		printf("animsOffset = %lld\n", movie.animsOffset);
		printf("stringsOffset = %lld\n", movie.stringsOffset);
		for (int i = 0; i < movie.numActors; i++) {
			printf("\n== ACTOR %d ==\n\n", i);
			print(movie.pActors[i]);
			print(movie.pAnims[i]);
		}
	}

	void output(ofstream& stream, long long offset, BinImageAnimation& Anim) {
		AnimIO AnimOut;
		Anim.animOffset = offset;
		AnimOut.mHasColor = Anim.mHasColor;
		AnimOut.mHasAlpha = Anim.mHasAlpha;
		AnimOut.mHasSound = Anim.mHasSound;
		AnimOut.mHasTransform = Anim.mHasTransform;
		AnimOut.mNumTransforms = Anim.mNumTransforms;
		AnimOut.mNumFrames = Anim.mNumFrames;
		AnimOut.transformTypeOffset = Anim.transformTypeOffset;
		AnimOut.frameTimesOffset = Anim.frameTimesOffset;
		AnimOut.xformFramesOffset = Anim.xformFramesOffset;
		AnimOut.alphaFramesOffset = Anim.alphaFramesOffset;
		AnimOut.colorFramesOffset = Anim.colorFramesOffset;
		AnimOut.soundFramesOffset = Anim.soundFramesOffset;
		AnimOut.stringTableOffset = Anim.stringTableOffset;
		stream.seekp(offset, ios::beg);
		stream.write((char*)&AnimOut, sizeof(AnimIO));
		//write framTimes
		stream.seekp(Anim.animOffset + Anim.frameTimesOffset, ios::beg);
		for (int i = 0; i < Anim.mNumFrames; i++)
			stream.write((char*)(Anim.mFrameTimes + i), sizeof(float));

		keyframeIO keyOut;
		stream.seekp(Anim.animOffset + Anim.transformTypeOffset, ios::beg);
		for (int i = 0; i < Anim.mNumTransforms; i++)
			stream.write((char*)(Anim.mTransformTypes + i), sizeof(TransformType));

		//write Transforms
		if (Anim.mHasTransform) {
			stream.seekp(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumTransforms; i++)
				stream.write((char*)(Anim.TransFrameOffset + i), sizeof(long long));

			//write xformFramesOffset
			stream.seekp(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++) {
				stream.seekp(Anim.animOffset + Anim.TransFrameOffset[i], ios::beg);
				for (int j = 0; j < Anim.mNumFrames; j++) {
					stream.write((char*)(Anim.TransSingleFrameOffset[i] + j), sizeof(long long));
				}
				for (int j = 0; j < Anim.mNumFrames; j++)
					if (Anim.mXformFrames[i][j].isValid) {
						keyOut.alpha = Anim.mXformFrames[i][j].alpha;
						keyOut.angle = Anim.mXformFrames[i][j].angle;
						keyOut.color = Anim.mXformFrames[i][j].color;
						keyOut.interpolation = Anim.mXformFrames[i][j].interpolation;
						keyOut.nextFrameIndex = Anim.mXformFrames[i][j].nextFrameIndex;
						keyOut.soundStrIdx = Anim.mXformFrames[i][j].soundStrIdx;
						keyOut.x = Anim.mXformFrames[i][j].x;
						keyOut.y = Anim.mXformFrames[i][j].y;
						stream.seekp(Anim.TransSingleFrameOffset[i][j] + Anim.animOffset, ios::beg);
						stream.write((char*)&keyOut, sizeof(keyframeIO));
					}
				for (int j = 0; j < Anim.mNumFrames; j++)
					if (Anim.mXformFrames[i][j].soundStrIdx)
						writeStr(stream, Anim.mXformFrames[i][j].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mXformFrames[i][j].soundString);
			}
		}


		//writeAlpha
		if (Anim.mHasAlpha) {
			stream.seekp(Anim.animOffset + Anim.alphaFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.write((char*)(Anim.AlphaOffset + i), sizeof(long long));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].isValid) {
					keyOut.alpha = Anim.mAlphaFrames[i].alpha;
					keyOut.angle = Anim.mAlphaFrames[i].angle;
					keyOut.color = Anim.mAlphaFrames[i].color;
					keyOut.interpolation = Anim.mAlphaFrames[i].interpolation;
					keyOut.nextFrameIndex = Anim.mAlphaFrames[i].nextFrameIndex;
					keyOut.soundStrIdx = Anim.mAlphaFrames[i].soundStrIdx;
					keyOut.x = Anim.mAlphaFrames[i].x;
					keyOut.y = Anim.mAlphaFrames[i].y;
					stream.seekp(Anim.animOffset + Anim.AlphaOffset[i], ios::beg);
					stream.write((char*)&keyOut, sizeof(keyframeIO));
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].soundStrIdx)
					writeStr(stream, Anim.mAlphaFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mAlphaFrames[i].soundString);

		}

		//writeColor
		if (Anim.mHasColor) {
			stream.seekp(Anim.animOffset + Anim.colorFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.write((char*)(Anim.ColorOffset + i), sizeof(long long));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].isValid) {
					keyOut.alpha = Anim.mColorFrames[i].alpha;
					keyOut.angle = Anim.mColorFrames[i].angle;
					keyOut.color = Anim.mColorFrames[i].color;
					keyOut.interpolation = Anim.mColorFrames[i].interpolation;
					keyOut.nextFrameIndex = Anim.mColorFrames[i].nextFrameIndex;
					keyOut.soundStrIdx = Anim.mColorFrames[i].soundStrIdx;
					keyOut.x = Anim.mColorFrames[i].x;
					keyOut.y = Anim.mColorFrames[i].y;
					stream.seekp(Anim.animOffset + Anim.ColorOffset[i], ios::beg);
					stream.write((char*)&keyOut, sizeof(keyframeIO));
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].soundStrIdx)
					writeStr(stream, Anim.mColorFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mColorFrames[i].soundString);

		}

		//writeSound
		if (Anim.mHasSound) {
			stream.seekp(Anim.animOffset + Anim.soundFramesOffset, ios::beg);
			for (int i = 0; i < Anim.mNumFrames; i++)
				stream.write((char*)(Anim.SoundOffset + i), sizeof(long long));
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].isValid) {
					keyOut.alpha = Anim.mSoundFrames[i].alpha;
					keyOut.angle = Anim.mSoundFrames[i].angle;
					keyOut.color = Anim.mSoundFrames[i].color;
					keyOut.interpolation = Anim.mSoundFrames[i].interpolation;
					keyOut.nextFrameIndex = Anim.mSoundFrames[i].nextFrameIndex;
					keyOut.soundStrIdx = Anim.mSoundFrames[i].soundStrIdx;
					keyOut.x = Anim.mSoundFrames[i].x;
					keyOut.y = Anim.mSoundFrames[i].y;
					stream.seekp(Anim.animOffset + Anim.SoundOffset[i], ios::beg);
					stream.write((char*)&keyOut, sizeof(keyframeIO));
				}
			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].soundStrIdx)
					writeStr(stream, Anim.mSoundFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset, Anim.mSoundFrames[i].soundString);

		}

	}

	void output(ofstream& stream, long long offset, BinMovie& Movie) {
		stream.seekp(offset, ios::beg);
		MovieIO MovieOut;

		MovieOut.length = Movie.length;
		MovieOut.numActors = Movie.numActors;
		AnimIO* AnimOut = new AnimIO[Movie.numActors];
		ActorIO* ActorOut = new ActorIO[Movie.numActors];
		MovieOut.actorsOffset = Movie.actorsOffset;
		MovieOut.animsOffset = Movie.animsOffset;
		MovieOut.stringsOffset = Movie.stringsOffset;

		stream.write((char*)&MovieOut, sizeof(MovieIO));

		//write Actors
		stream.seekp(offset + Movie.actorsOffset, ios::beg);
		for (int i = 0; i < Movie.numActors; i++) {
			ActorOut[i].mType = Movie.pActors[i].mType;
			ActorOut[i].mImageStrIdx = Movie.pActors[i].mImageStrIdx;
			ActorOut[i].mLabelTextStrIdx = Movie.pActors[i].mLabelTextStrIdx;
			ActorOut[i].mFontStrIdx = Movie.pActors[i].mFontStrIdx;
			ActorOut[i].mLabelMaxWidth = Movie.pActors[i].mLabelMaxWidth;
			ActorOut[i].mLabelWrapWidth = Movie.pActors[i].mLabelWrapWidth;
			ActorOut[i].mLabelJustification = Movie.pActors[i].mLabelJustification;
			ActorOut[i].mDepth = Movie.pActors[i].mDepth;
			stream.write((char*)(ActorOut + i), sizeof(ActorIO));
		}
		for (int i = 0; i < MovieOut.numActors; i++) {
			writeStr(stream, Movie.stringsOffset + Movie.pActors[i].mImageStrIdx, Movie.pActors[i].ImageStr);
			writeStr(stream, Movie.stringsOffset + Movie.pActors[i].mLabelTextStrIdx, Movie.pActors[i].LabelTextStr);
			writeStr(stream, Movie.stringsOffset + Movie.pActors[i].mFontStrIdx, Movie.pActors[i].FontStr);
		}

		//read Anim offset
		stream.seekp(offset + MovieOut.animsOffset, ios::beg);
		for (int i = 0; i < MovieOut.numActors; i++)
			stream.write((char*)&(Movie.pAnims[i].animOffset), sizeof(long long));
		for (int i = 0; i < MovieOut.numActors; i++) {
			output(stream, Movie.pAnims[i].animOffset, Movie.pAnims[i]);
		}
	}

	int alloc(int& stringOffset, vector<string>& strList, vector<int>& pos, string str) {
		if (str == "")
			return 0;
		int currentLoc = stringOffset;
		for (int i = 0; i < strList.size(); i++)
			if (str == strList[i])
				return pos[i];
		stringOffset += str.length() + 1;
		strList.push_back(str);
		pos.push_back(currentLoc);
		return currentLoc;
	}

	long long deploy(long long& GlobOffset, int stringOffset) {
		long long currentPos = GlobOffset;
		GlobOffset += stringOffset + 3;
		return currentPos;
	}

	void alloc(long long& GlobOffset, BinImageAnimation& Anim) {
		vector<string>strList;
		vector<int>pos;
		int strOffset = 4;
		long long selfOffset = GlobOffset;

		//stream.seekg(offset, ios::beg);
		Anim.animOffset = GlobOffset;														//offset=0

		//stream.read((char*)&AnimIn, sizeof(AnimIO));
		GlobOffset += sizeof(AnimIO);

		//stream.seekg(Anim.animOffset + Anim.transformTypeOffset, ios::beg);
		Anim.transformTypeOffset = GlobOffset - Anim.animOffset;							//offset=52

		//for (int i = 0; i < Anim.mNumTransforms; i++)
			//stream.read((char*)(Anim.mTransformTypes  + i), sizeof(TransformType));
		GlobOffset += sizeof(TransformType) * Anim.mNumTransforms;

		//stream.seekg(Anim.animOffset + Anim.frameTimesOffset, ios::beg);
		Anim.frameTimesOffset = GlobOffset - Anim.animOffset;								//offset=64

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.mFrameTimes + i), sizeof(float));
		GlobOffset += sizeof(float) * Anim.mNumFrames;

		//stream.seekg(Anim.animOffset + Anim.xformFramesOffset, ios::beg);
		Anim.xformFramesOffset = GlobOffset - Anim.animOffset;								//offset=72

		//for (int i = 0; i < Anim.mNumTransforms; i++)
			//stream.read((char*)(Anim.TransFrameOffset + i), sizeof(int));
		GlobOffset += sizeof(long long) * Anim.mNumTransforms;									//offset=84

		//getTransform
		if (Anim.mHasTransform) {

			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++) {

				//stream.seekg(Anim.animOffset + Anim.TransFrameOffset[i], ios::beg);
				Anim.TransFrameOffset[i] = GlobOffset - Anim.animOffset;

				//for (int j = 0; j < Anim.mNumFrames; j++)
					//stream.read((char*)(Anim.TransSingleFrameOffset[i] + j), sizeof(int));
				GlobOffset += sizeof(long long) * Anim.mNumFrames;
			}
		}

		//stream.seekg(Anim.animOffset + Anim.alphaFramesOffset, ios::beg);
		Anim.alphaFramesOffset = GlobOffset - Anim.animOffset;

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.AlphaOffset + i), sizeof(int));
		GlobOffset += sizeof(long long) * Anim.mNumFrames;										//offset=108

		//stream.seekg(Anim.animOffset + Anim.colorFramesOffset, ios::beg);
		Anim.colorFramesOffset = GlobOffset - Anim.animOffset;

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.ColorOffset + i), sizeof(int));
		GlobOffset += sizeof(long long) * Anim.mNumFrames;										//offset=116

		//stream.seekg(Anim.animOffset + Anim.soundFramesOffset, ios::beg);
		Anim.soundFramesOffset = GlobOffset - Anim.animOffset;

		//for (int i = 0; i < Anim.mNumFrames; i++)
			//stream.read((char*)(Anim.SoundOffset + i), sizeof(int));
		GlobOffset += sizeof(long long) * Anim.mNumFrames;										//offset=124

		//string assign begin
		if (Anim.mHasTransform) {
			for (int i = 0; i < Anim.mNumTransforms; i++)
				for (int j = 0; j < Anim.mNumFrames; j++)
					//if (Anim.mXformFrames[i][j].soundStrIdx)
						//Anim.mXformFrames[i][j].soundString = readStr(stream, Anim.mXformFrames[i][j].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
					Anim.mXformFrames[i][j].soundStrIdx = alloc(strOffset, strList, pos, Anim.mXformFrames[i][j].soundString);
		}

		if (Anim.mHasAlpha) {
			for (int i = 0; i < Anim.mNumFrames; i++)
				//if (Anim.mAlphaFrames[i].soundStrIdx)
					//Anim.mAlphaFrames[i].soundString = readStr(stream, Anim.mAlphaFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				Anim.mAlphaFrames[i].soundStrIdx = alloc(strOffset, strList, pos, Anim.mAlphaFrames[i].soundString);
		}

		if (Anim.mHasColor) {
			for (int i = 0; i < Anim.mNumFrames; i++)
				//if (Anim.mColorFrames[i].soundStrIdx)
					//Anim.mColorFrames[i].soundString = readStr(stream, Anim.mColorFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				Anim.mColorFrames[i].soundStrIdx = alloc(strOffset, strList, pos, Anim.mColorFrames[i].soundString);
		}

		if (Anim.mHasSound) {
			for (int i = 0; i < Anim.mNumFrames; i++)
				//if (Anim.mSoundFrames[i].soundStrIdx)
					//Anim.mSoundFrames[i].soundString = readStr(stream, Anim.mSoundFrames[i].soundStrIdx + Anim.animOffset + Anim.stringTableOffset);
				Anim.mSoundFrames[i].soundStrIdx = alloc(strOffset, strList, pos, Anim.mSoundFrames[i].soundString);
		}

		Anim.stringTableOffset = deploy(GlobOffset, strOffset) - Anim.animOffset;
		//string assign end

		if (Anim.mHasTransform) {
			//scale/rotation/translate
			for (int i = 0; i < Anim.mNumTransforms; i++)
				for (int j = 0; j < Anim.mNumFrames; j++)
					if (Anim.mXformFrames[i][j].isValid) {
						//stream.seekg(Anim.TransSingleFrameOffset[i][j] + Anim.animOffset, ios::beg);
						Anim.TransSingleFrameOffset[i][j] = GlobOffset - Anim.animOffset;

						//stream.read((char*)&keyIn, sizeof(keyframeIO));
						GlobOffset += sizeof(keyframeIO);
					}
					else {
						Anim.TransSingleFrameOffset[i][j] = 0;
					}
		}

		//getAlpha
		if (Anim.mHasAlpha) {

			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mAlphaFrames[i].isValid) {
					//stream.seekg(Anim.animOffset + Anim.AlphaOffset[i], ios::beg);
					Anim.AlphaOffset[i] = GlobOffset - Anim.animOffset;

					//stream.read((char*)&keyIn, sizeof(keyframeIO));
					GlobOffset += sizeof(keyframeIO);
				}
				else {
					Anim.AlphaOffset[i] = 0;
				}
		}

		//getColor
		if (Anim.mHasColor) {

			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mColorFrames[i].isValid) {
					//stream.seekg(Anim.animOffset + Anim.ColorOffset[i], ios::beg);
					Anim.ColorOffset[i] = GlobOffset - Anim.animOffset;

					//stream.read((char*)&keyIn, sizeof(keyframeIO));
					GlobOffset += sizeof(keyframeIO);
				}
				else {
					Anim.ColorOffset[i] = 0;
				}
		}
		//getSound
		if (Anim.mHasSound) {

			for (int i = 0; i < Anim.mNumFrames; i++)
				if (Anim.mSoundFrames[i].isValid) {
					//stream.seekg(Anim.animOffset + Anim.SoundOffset[i], ios::beg);
					Anim.SoundOffset[i] = GlobOffset - Anim.animOffset;

					//stream.read((char*)&keyIn, sizeof(keyframeIO));
					GlobOffset += sizeof(keyframeIO);

				}
				else {
					Anim.SoundOffset[i] = 0;
				}
		}
	}

	void alloc(long long& GlobOffset, BinMovie& Movie) {
		vector<string>strList;
		vector<int>pos;
		int strOffset = 4;
		long long offset;

		//stream.seekg(offset, ios::beg);
		offset = GlobOffset;

		//stream.read((char*)&MovieIn, sizeof(MovieIO));
		GlobOffset += sizeof(MovieIO);

		//read Actors
		//stream.seekg(offset + MovieIn.actorsOffset, ios::beg);
		Movie.actorsOffset = GlobOffset - offset;

		//for (int i = 0; i < MovieIn.numActors; i++)
			//stream.read((char*)(ActorIn + i), sizeof(ActorIO));
		GlobOffset += sizeof(ActorIO) * Movie.numActors;

		//read Anim offset
		//stream.seekg(offset + MovieIn.animsOffset, ios::beg);
		Movie.animsOffset = GlobOffset - offset;

		for (int i = 0; i < Movie.numActors; i++) {
			//Movie.pActors[i].ImageStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mImageStrIdx);
			Movie.pActors[i].mImageStrIdx = alloc(strOffset, strList, pos, Movie.pActors[i].ImageStr);

			//Movie.pActors[i].LabelTextStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mLabelTextStrIdx);
			Movie.pActors[i].mLabelTextStrIdx = alloc(strOffset, strList, pos, Movie.pActors[i].LabelTextStr);

			//Movie.pActors[i].FontStr = readStr(stream, Movie.stringsOffset + Movie.pActors[i].mFontStrIdx);
			Movie.pActors[i].mFontStrIdx = alloc(strOffset, strList, pos, Movie.pActors[i].FontStr);
		}

		//for (int i = 0; i < MovieIn.numActors; i++)
			//stream.read((char*)(AnimOffset + i), sizeof(int));
		GlobOffset += sizeof(long long) * Movie.numActors;

		Movie.stringsOffset = deploy(GlobOffset, strOffset) - offset;

		for (int i = 0; i < Movie.numActors; i++) {
			//ReadAnim(stream, AnimOffset[i], Movie.pAnims[i]);
			Movie.pAnims[i].animOffset = GlobOffset;
			alloc(GlobOffset, Movie.pAnims[i]);
		}


	}

}

namespace tool {

	bool match_final(string str, const char* substr) {
		return str.rfind(substr) == str.length() - strlen(substr);
	}

	string replace_final(string origin, const char* replacer, const char* replacee) {
		origin.replace(origin.rfind(replacee), strlen(replacer), replacer);
		return origin;
	}

	newFormat::keyframe toNew(oldFormat::keyframe oldKey) {
		newFormat::keyframe newKey;
		newKey.isValid = oldKey.isValid;
		newKey.x = oldKey.x;
		newKey.y = oldKey.y;
		newKey.angle = oldKey.angle;
		newKey.alpha = oldKey.alpha;
		newKey.color = oldKey.color;
		newKey.nextFrameIndex = oldKey.nextFrameIndex;
		newKey.interpolation = newFormat::InterpolationType(oldKey.interpolation);
		newKey.soundString = oldKey.soundString;
		return newKey;
	}

	newFormat::BinImageAnimation toNew(oldFormat::BinImageAnimation oldAnim) {
		newFormat::BinImageAnimation newAnim;
		newAnim.mHasColor = oldAnim.mHasColor;
		newAnim.mHasAlpha = oldAnim.mHasAlpha;
		newAnim.mHasSound = oldAnim.mHasSound;
		newAnim.mHasTransform = oldAnim.mHasTransform;

		newAnim.mNumTransforms = oldAnim.mNumTransforms;
		newAnim.mNumFrames = oldAnim.mNumFrames;

		newAnim.mTransformTypes = new newFormat::TransformType[oldAnim.mNumTransforms];
		for (int i = 0; i < oldAnim.mNumTransforms; i++) {
			newAnim.mTransformTypes[i] = newFormat::TransformType(oldAnim.mTransformTypes[i]);
			if (oldAnim.mHasTransform) {
				newAnim.TransSingleFrameOffset[i] = new long long[oldAnim.mNumFrames];
				newAnim.mXformFrames[i] = new newFormat::keyframe[oldAnim.mNumFrames];
				for (int j = 0; j < oldAnim.mNumFrames; j++)
					newAnim.mXformFrames[i][j] = toNew(oldAnim.mXformFrames[i][j]);
			}
		}

		newAnim.mFrameTimes = new float[oldAnim.mNumFrames];
		for (int i = 0; i < oldAnim.mNumFrames; i++)
			newAnim.mFrameTimes[i] = oldAnim.mFrameTimes[i];

		newAnim.AlphaOffset = new long long[oldAnim.mNumFrames];
		if (oldAnim.mHasAlpha) {
			newAnim.mAlphaFrames = new newFormat::keyframe[oldAnim.mNumFrames];
			for (int i = 0; i < oldAnim.mNumFrames; i++)
				newAnim.mAlphaFrames[i] = toNew(oldAnim.mAlphaFrames[i]);
		}

		newAnim.ColorOffset = new long long[oldAnim.mNumFrames];
		if (oldAnim.mHasColor) {
			newAnim.mColorFrames = new newFormat::keyframe[oldAnim.mNumFrames];
			for (int i = 0; i < oldAnim.mNumFrames; i++)
				newAnim.mColorFrames[i] = toNew(oldAnim.mColorFrames[i]);
		}

		newAnim.SoundOffset = new long long[oldAnim.mNumFrames];
		if (oldAnim.mHasSound) {
			newAnim.mSoundFrames = new newFormat::keyframe[oldAnim.mNumFrames];
			for (int i = 0; i < oldAnim.mNumFrames; i++)
				newAnim.mSoundFrames[i] = toNew(oldAnim.mSoundFrames[i]);
		}
		return newAnim;
	}

	newFormat::BinActor toNew(oldFormat::BinActor oldActor) {
		newFormat::BinActor newActor;
		newActor.mType = newFormat::ActorType(oldActor.mType);
		newActor.ImageStr = oldActor.ImageStr;
		newActor.LabelTextStr = oldActor.LabelTextStr;
		newActor.FontStr = oldActor.FontStr;
		newActor.mLabelMaxWidth = oldActor.mLabelMaxWidth;
		newActor.mLabelWrapWidth = oldActor.mLabelWrapWidth;
		newActor.mLabelJustification = newFormat::AlignmentH(oldActor.mLabelJustification);
		newActor.mDepth = oldActor.mDepth;
		return newActor;
	}

	newFormat::BinMovie toNew(oldFormat::BinMovie oldMovie) {
		newFormat::BinMovie newMovie;
		newMovie.length = oldMovie.length;
		newMovie.numActors = oldMovie.numActors;
		newMovie.pActors = new newFormat::BinActor[oldMovie.numActors];
		for (int i = 0; i < oldMovie.numActors; i++)
			newMovie.pActors[i] = toNew(oldMovie.pActors[i]);
		newMovie.pAnims = new newFormat::BinImageAnimation[oldMovie.numActors];
		for (int i = 0; i < oldMovie.numActors; i++)
			newMovie.pAnims[i] = toNew(oldMovie.pAnims[i]);
		return newMovie;
	}

	oldFormat::keyframe toOld(newFormat::keyframe newKey) {
		oldFormat::keyframe oldKey;
		oldKey.x = newKey.x;
		oldKey.y = newKey.y;
		oldKey.angle = newKey.angle;
		oldKey.alpha = newKey.alpha;
		oldKey.color = newKey.color;
		oldKey.nextFrameIndex = newKey.nextFrameIndex;
		oldKey.interpolation = oldFormat::InterpolationType(newKey.interpolation);
		oldKey.soundString = newKey.soundString;
		return oldKey;
	}

	oldFormat::BinImageAnimation toOld(newFormat::BinImageAnimation newAnim) {
		oldFormat::BinImageAnimation oldAnim;
		oldAnim.mHasColor = newAnim.mHasColor;
		oldAnim.mHasAlpha = newAnim.mHasAlpha;
		oldAnim.mHasSound = newAnim.mHasSound;
		oldAnim.mHasTransform = newAnim.mHasTransform;

		oldAnim.mNumTransforms = newAnim.mNumTransforms;
		oldAnim.mNumFrames = newAnim.mNumFrames;

		oldAnim.mTransformTypes = new oldFormat::TransformType[newAnim.mNumTransforms];
		for (int i = 0; i < newAnim.mNumTransforms; i++) {
			oldAnim.mTransformTypes[i] = oldFormat::TransformType(newAnim.mTransformTypes[i]);
			if (newAnim.mHasTransform) {
				oldAnim.TransSingleFrameOffset[i] = new int[newAnim.mNumFrames];
				oldAnim.mXformFrames[i] = new oldFormat::keyframe[newAnim.mNumFrames];
				for (int j = 0; j < newAnim.mNumFrames; j++)
					oldAnim.mXformFrames[i][j] = toOld(newAnim.mXformFrames[i][j]);
			}
		}

		oldAnim.mFrameTimes = new float[newAnim.mNumFrames];
		for (int i = 0; i < newAnim.mNumFrames; i++)
			oldAnim.mFrameTimes[i] = newAnim.mFrameTimes[i];

		oldAnim.AlphaOffset = new int[newAnim.mNumFrames];
		if (newAnim.mHasAlpha) {
			oldAnim.mAlphaFrames = new oldFormat::keyframe[newAnim.mNumFrames];
			for (int i = 0; i < newAnim.mNumFrames; i++)
				oldAnim.mAlphaFrames[i] = toOld(newAnim.mAlphaFrames[i]);
		}

		oldAnim.ColorOffset = new int[newAnim.mNumFrames];
		if (newAnim.mHasColor) {
			oldAnim.mColorFrames = new oldFormat::keyframe[newAnim.mNumFrames];
			for (int i = 0; i < newAnim.mNumFrames; i++)
				oldAnim.mColorFrames[i] = toOld(newAnim.mColorFrames[i]);
		}

		oldAnim.SoundOffset = new int[newAnim.mNumFrames];
		if (newAnim.mHasSound) {
			oldAnim.mSoundFrames = new oldFormat::keyframe[newAnim.mNumFrames];
			for (int i = 0; i < newAnim.mNumFrames; i++)
				oldAnim.mSoundFrames[i] = toOld(newAnim.mSoundFrames[i]);
		}
		return oldAnim;
	}

	oldFormat::BinActor toOld(newFormat::BinActor newActor) {
		oldFormat::BinActor oldActor;
		oldActor.mType = oldFormat::ActorType(newActor.mType);
		oldActor.ImageStr = newActor.ImageStr;
		oldActor.LabelTextStr = newActor.LabelTextStr;
		oldActor.FontStr = newActor.FontStr;
		oldActor.mLabelMaxWidth = newActor.mLabelMaxWidth;
		oldActor.mLabelWrapWidth = newActor.mLabelWrapWidth;
		oldActor.mLabelJustification = oldFormat::AlignmentH(newActor.mLabelJustification);
		oldActor.mDepth = newActor.mDepth;
		return oldActor;
	}

	oldFormat::BinMovie toOld(newFormat::BinMovie newMovie) {
		oldFormat::BinMovie oldMovie;
		oldMovie.length = newMovie.length;
		oldMovie.numActors = newMovie.numActors;
		oldMovie.pActors = new oldFormat::BinActor[newMovie.numActors];
		for (int i = 0; i < newMovie.numActors; i++)
			oldMovie.pActors[i] = toOld(newMovie.pActors[i]);
		oldMovie.pAnims = new oldFormat::BinImageAnimation[newMovie.numActors];
		for (int i = 0; i < newMovie.numActors; i++)
			oldMovie.pAnims[i] = toOld(newMovie.pAnims[i]);
		return oldMovie;
	}

	bool FileExists(string& name) {
		ifstream f(name.c_str());
		return f.good();
	}

	FILETIME getTime(string FileName) {
		USES_CONVERSION;
		HANDLE FileHandle = CreateFile(A2T(FileName.c_str()), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		FILETIME Create, LastWrite, LastAccess;
		GetFileTime(FileHandle, &Create, &LastAccess, &LastWrite);
		return LastWrite;
	}

	//correct With 1
	int cmpModifyTime(string oldName, string newName) {
		FILETIME oldTime = getTime(oldName),
			newTime = getTime(newName);
		ULONGLONG oldTimeUll = (((ULONGLONG)oldTime.dwHighDateTime) << 32) + oldTime.dwLowDateTime,
			newTimeUll = (((ULONGLONG)newTime.dwHighDateTime) << 32) + newTime.dwLowDateTime;
		if (oldTimeUll > newTimeUll)
			return -1;
		if (oldTimeUll < newTimeUll)
			return 1;
		return 0;
	}


}
//#define debug
//#define printStructOnly
using namespace tool;

int main(int argn, char* args[])
{
	string str, outfile;

	system("title World Of Goo Movie Binary Transformer");
	printf("This is a fan-made and open-source file for Mod Creation\n\n");

	if (argn != 2) {
		printf("Invalid Arguments! Please Drag Any Movie Files to Convert\n");
		system("pause");
		return 1;
	}
	else
		str = args[1];
#ifndef debug
	try {
#endif
		if (match_final(str, ".movie.binltl")) {
			printf("File matched .movie.binltl (old version movie)\n\n");
			oldFormat::BinMovie movie;
			ifstream file(str, ios::binary);
			if (!file.is_open()) {
				cerr << "Failed to open file: " << str << endl;
				return 3;
			}
			ReadMovie(file, 0, movie);

#ifdef printStructOnly
			print(movie);
			system("pause");
			return 0;
#endif

			outfile = replace_final(str, ".movie.binuni", ".movie.binltl");

			if ((!FileExists(outfile)) || cmpModifyTime(outfile, str) == 1) {

				newFormat::BinMovie movieOut;
				movieOut = toNew(movie);

				long long offset = 0;
				alloc(offset, movieOut);

				//print(movieOut);
				ofstream out(outfile, ios::binary);
				output(out, 0, movieOut);
				printf("Output successfully into %s\n", outfile.c_str());
			}
			else {
				printf("File %s Exists and seems to be newer!\n", outfile.c_str());
				return 1;
			}
		}

		else if (match_final(str, ".anim.binltl")) {
			printf("File matched .anim.binltl (old version animation)\n\n");
			oldFormat::BinImageAnimation anim;
			ifstream file(str, ios::binary);
			if (!file.is_open()) {
				cerr << "Failed to open file: " << str << endl;
				return 3;
			}
			ReadAnim(file, 0, anim);

#ifdef printStructOnly
			print(anim);
			system("pause");
			return 0;
#endif

			outfile = replace_final(str, ".anim.binuni", ".anim.binltl");
			if ((!FileExists(outfile)) || cmpModifyTime(outfile, str) == 1) {

				newFormat::BinImageAnimation animOut;
				animOut = toNew(anim);

				long long offset = 0;
				alloc(offset, animOut);

				//print(movie);
				ofstream out(outfile, ios::binary);
				output(out, 0, animOut);
				printf("Output successfully into %s\n", outfile.c_str());
			}
			else {
				printf("File %s Exists and seems to be newer!\n", outfile.c_str());
				return 1;
			}

		}

		else if (match_final(str, ".movie.binuni")) {
			printf("File matched .anim.binuni (new version movie)\n\n");
			newFormat::BinMovie movie;
			ifstream file(str, ios::binary);
			if (!file.is_open()) {
				cerr << "Failed to open file: " << str << endl;
				return 3;
			}
			ReadMovie(file, 0, movie);
#ifdef printStructOnly
			print(movie);
			system("pause");
			return 0;
#endif

			outfile = replace_final(str, ".movie.binltl", ".movie.binuni");

			if ((!FileExists(outfile)) || cmpModifyTime(outfile, str) == 1) {

				oldFormat::BinMovie movieOut;
				movieOut = toOld(movie);

				int offset = 0;
				alloc(offset, movieOut);

				//print(movieOut);
				ofstream out(outfile, ios::binary);
				output(out, 0, movieOut);
				printf("Output successfully into %s\n", outfile.c_str());
				return 1;
			}
			else {
				printf("File %s Exists and seems to be newer!\n", outfile.c_str());
			}
		}

		else if (match_final(str, ".anim.binuni")) {
			printf("File matched .anim.binuni (new version animation)\n\n");
			newFormat::BinImageAnimation anim;
			ifstream file(str, ios::binary);
			if (!file.is_open()) {
				cerr << "Failed to open file: " << str << endl;
				return 3;
			}
			ReadAnim(file, 0, anim);
#ifdef printStructOnly
			print(anim);
			system("pause");
			return 0;
#endif

			outfile = replace_final(str, ".anim.binltl", ".anim.binuni");
			if ((!FileExists(outfile)) || cmpModifyTime(outfile, str) == 1) {

				oldFormat::BinImageAnimation animOut;
				animOut = toOld(anim);

				int offset = 0;
				alloc(offset, animOut);

				//print(movie);
				ofstream out(outfile, ios::binary);
				output(out, 0, animOut);
				printf("Output successfully into %s\n", outfile.c_str());
			}
			else {
				printf("File %s Exists and seems to be newer!\n", outfile.c_str());
				return 1;
			}
		}
		else {
			printf("File %s is not a valid movie file!\n", str.c_str());
		}
#ifndef debug
	}
	catch (std::bad_array_new_length) {
		printf("Wrong File Format!\n");
		system("pause");
		return 2;
	};
#endif
	system("pause");
	return 0;
}

