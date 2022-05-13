#pragma once
class Piece
{
public:
	int type;		// rodzaj figury
	float posX;		// pozycja X na szachownicy
	float posY;		// pozycja Y (do podnoszenia opuszczania)
	float posZ;		// pozycja Z na szachownicy
	float destruct; // stopieñ zniszczenia
	bool color;		// kolor figury (0 = bia³y, 1 = czarny)

	Piece(int typ, float X, float Y, float Z, bool col) {
		type = typ;
		posX = X;
		posY = Y;
		posZ = Z;
		color = col;
		destruct = 0;
	}
	Piece() = default;

};