#pragma once
#include "bits-stdc++.h"
#include "Terminal.hpp"
#include <bitset> 
#include "Match.hpp"
#include "MemoKey.hpp"
#include "StringUtils.hpp"

class CharSet : public Terminal
{
private:
	bitset<8> chars;
	bitset<8> invertedChars;
	//То что выше вроде как аналог их варианта, но у них почему-то это безразмерные множества.

public:
	TypesOfClauses TypeOfClause = TypesOfClauses::CharSet;
	CharSet()
	{}

	CharSet(vector<char> chars) : Terminal()
	{
		//У них первой строчкой идёт конструктор супер, я не совсем понял зачем.
		for (int i = 0; i < chars.size(); i++)
		{
			this->chars.set(chars[i]);
		}
	}

	CharSet(vector<CharSet*> CharSets) : Terminal()
	{
		if (CharSets.size() == 0)
		{
			cout << "Must provide at least one CharSet";
			abort();
		}
		for (auto charSet : CharSets)
		{
			for (int i = charSet->chars.nextSetBit(0); i >= 0; i = charSet->chars._Find_next(i + 1)) // nextSetBit возвращает индекс следующего бита, я так понял аналога в std::bitset нет, наверное надо писать самостоятельно, так как Find_next из подключенного хэдера, который почему то подключается только как локальный не работает.
			{
				this->chars.set(i);
			}
			for (int i = charSet->invertedChars.nextSetBit(0); i >= 0; i = charSet->invertedChars.nextSetBit(i + 1)) 
			{
				this->invertedChars.set(i);
			}
		}
	}

	CharSet(bitset<8> chars) : Terminal()
	{
		if (chars.cardinality() == 0)
		{
			cout << "Must provide at least one CharSet";
			abort();
		}
		this->chars = chars;
	}

	CharSet* invert() 
	{
		auto tmp = chars;
		chars = invertedChars;
		invertedChars = tmp;
		toStringCached = "";
		return this;
	}

	void determineWhetherCanMatchZeroChars() 
	{}

	Match* match(MemoTable* memoTable, MemoKey* memoKey, string input) 
	{
		if (memoKey->startPos < input.length()) 
		{
			char c = input[memoKey->startPos];
			bitset<8> b(c);
			if ((chars != NULL && (chars == b)) || (invertedChars != NULL && !(invertedChars == b))) 
			{
				Match* mast = new Match(memoKey, /* len = */ 1, Match::NO_SUBCLAUSE_MATCHES);
				return mast;
			}
		}
		return nullptr;
	}

	void toString(bitset<8> chars, int cardinality, bool inverted, string buf) {
		bool isSingleChar = !inverted && cardinality == 1;
		if (isSingleChar) {
			char c = (char)chars.nextSetBit(0);
			buf.append("'");
			buf.append(StringUtils::escapeQuotedChar(c));
			buf.append("'");
		}
		else {
			buf.append("[");
			if (inverted) {
				buf.append(" ^ ");
			}
			for (int i = chars.nextSetBit(0); i >= 0; i = chars.nextSetBit(i + 1)) {
				buf.append(StringUtils::escapeCharRangeChar((char)i));
				if (i < chars.size() - 1 && chars.get(i + 1)) {
					// Contiguous char range
					int end = i + 2;
					while (end < chars.size() && chars.get(end)) {
						end++;
					}
					int numCharsSpanned = end - i;
					if (numCharsSpanned > 2) {
						buf.append(" - ");
					}
					buf.append(StringUtils::escapeCharRangeChar((char)(end - 1)));
					i = end - 1;
				}
			}
			buf.append("]");
		}
	}

	string toString() {
		if (toStringCached.empty()) {
			string buf;
			auto charsCardinality = chars == NULL ? 0 : chars.cardinality();
			auto invertedCharsCardinality = invertedChars == NULL ? 0 : invertedChars.cardinality();
			auto invertedAndNot = charsCardinality > 0 && invertedCharsCardinality > 0;
			if (invertedAndNot) {
				buf.append("(");
			}
			if (charsCardinality > 0) {
				toString(chars, charsCardinality, false, buf);
			}
			if (invertedAndNot) {
				buf.append(" | ");
			}
			if (invertedCharsCardinality > 0) {
				toString(invertedChars, invertedCharsCardinality, true, buf);
			}
			if (invertedAndNot) {
				buf.append(")");
			}
			toStringCached = buf;
		}
		return toStringCached;
	}

	//просто вывод, потом если нужно добавим
};