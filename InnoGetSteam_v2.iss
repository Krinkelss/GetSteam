;;---Модуль получения пути к профилю пользователя Steam---;;
;;----------для установки модификаций Steam-игр-----------;;
;;---------- Автор библиотеки - Krinkels и SotM-----------;;
;;----------Автор скрипта - Gnom (aka Лександер)----------;;
;;--------------Доработка скрипта - Shegorat--------------;;

[Setup]
AppName=MyApp
AppVerName=MyApp
;;*Дописываем нужный путь в папке профиля пользователа после {code:GetInstallDir}\
DefaultDirName={pf}\123
;;*Следующие строчки не менять и не удалять!
DirExistsWarning=no
UsePreviousAppDir=false
DisableDirPage=yes
UsePreviousSetupType=false
Uninstallable=false
AlwaysShowDirOnReadyPage=true
;;*Предидущие строчки не менять и не удалять!

[Files]
Source: GetSteam.dll; Flags: dontcopy;

[code]
var
  SteamPage: TWizardPage;
  SteamListBox: TListBox;
  SteamDesc: TLabel;
  
//Возвращает число ников
function Steam_GetNumAccounts(): integer; external 'Steam_GetNumAccounts@files:GetSteam.dll cdecl';
//Возвращает ник под номером N
function Steam_GetNickNameN(iAccount: Integer): PChar; external 'Steam_GetNickNameN@files:GetSteam.dll cdecl';
//Возвращает путь к нику под номером N
function Steam_GetProfilePath(iAccount: Integer): PChar; external 'Steam_GetProfilePath@files:GetSteam.dll cdecl';
//Возвращает активный ник
function Steam_GetActiveNickName(): PChar; external 'Steam_GetActiveNickName@files:GetSteam.dll cdecl';
//Если Steam не установлен TRUE
function Steam_GetError: Boolean; external 'Steam_GetError@files:GetSteam.dll cdecl';
//Если произошла какая то ошибка то возвращает текст ошибки
function Steam_GetErrorText: PChar; external 'Steam_GetErrorText@files:GetSteam.dll cdecl';

function GetSelected(): Integer;
var i: Integer;
begin
  Result:=-1; if (SteamlistBox.Items.Count=0) then Exit;
  for i:=0 to SteamlistBox.Items.Count-1 do
    if SteamListBox.Selected[i] then begin
      Result:= i;
      Break;
    end;
end;

procedure SetSelected(AList: TListBox; Index: Integer);
begin
  SendMessage(AList.Handle, $186, Index, 0);
end;

function GetInstallDir(): String;
var i: Integer;
begin
  i:= GetSelected;
  if i=-1 then begin
    Result:= ExpandConstant('{pf}\Steam');
    Exit;
  end;
  Result:= Steam_GetProfilePath(i+1);
end;

procedure NickOnClick(Sender: TObject);
begin
  WizardForm.DirEdit.Text:= GetInstallDir()+'\MyPlugin';
end;

procedure InitializeWizard();
var k: Integer;
begin
  SteamPage:= CreateCustomPage(wpWelcome, 'Steam-аккаунт', 'Выберите Steam-аккаунт для которого установить данное дополнение');
  
  SteamDesc:= Tlabel.Create(SteamPage);
  SteamDesc.AutoSize:= False;
  SteamDesc.WordWrap:= true;
  SteamDesc.Setbounds(ScaleX(0), ScaleY(0), ScaleX(415), ScaleY(30));
  SteamDesc.Caption:='Выберите один из нижеперечсленных Steam-аккаунтов. Нажмите кнопку "Далее" для продожения либо "Отмена" для выхода из программы установки.';
  SteamDesc.Parent:= SteamPage.Surface;
  
  SteamListBox:= TListBox.Create(SteamPage);
  SteamListBox.SetBounds(ScaleX(0), ScaleY(32),ScaleX(415), ScaleY(195));
  SteamListBox.OnClick:= @NickOnClick;
  SteamListBox.Parent:= SteamPage.Surface;
  
  log(intToStr(Steam_GetNumAccounts))
  for k:= 1 to Steam_GetNumAccounts() do begin
    SteamListBox.Items.Add('#'+IntToStr(k)+Padl(Trim(Steam_GetNickNameN(k)), 30)+Padl(Steam_GetProfilePath(k), 70));
    if (String(Steam_GetNickNameN(k))=String(Steam_GetActiveNickName)) then SetSelected(SteamListBox, k);
  end;
end;


function InitializeSetup(): Boolean;
begin
  if Steam_GetError then
		begin
			MsgBox('GetSteam.dll вернула ошибку: "'+Steam_GetErrorText()+'"'+#13#10+#13#10+'Продолжение установки невозможно!', mbCriticalError, mb_ok);
			Result:=False;
			Exit;
		end
			MsgBox(Steam_GetActiveNickName(), mbCriticalError, mb_ok);
  Result:= True;
end;

