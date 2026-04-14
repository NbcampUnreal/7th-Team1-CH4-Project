// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Network/UI/EDLobbyPlayerEntry.h"
#include "Components/TextBlock.h"

void UEDLobbyPlayerEntry::SetPlayerInfo(const FString& InNickname, int32 InTeamId, bool bInReady)
{
	if (NicknameText)
	{
		NicknameText->SetText(FText::FromString(InNickname));
	}

	if (TeamText)
	{
		FString TeamStr;
		switch (InTeamId)
		{
		case 0:  TeamStr = TEXT("미배정"); break;
		case 1:  TeamStr = TEXT("Team A"); break;
		case 2:  TeamStr = TEXT("Team B"); break;
		case 3:  TeamStr = TEXT("Team C"); break;
		default: TeamStr = FString::Printf(TEXT("Team %d"), InTeamId); break;
		}
		TeamText->SetText(FText::FromString(TeamStr));
	}

	if (ReadyText)
	{
		ReadyText->SetText(FText::FromString(bInReady ? TEXT("Ready") : TEXT("Not Ready")));
	}
}
