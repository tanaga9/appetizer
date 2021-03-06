unit Imaging;

interface

uses
	Windows, Graphics, PNGImage, SysUtils, Types;

const
 WS_EX_LAYERED = $80000;
 LWA_COLORKEY = 1;
 LWA_ALPHA    = 2;

type

  TPNGNineSlices = Array[0..8] of TPNGObject;

 TSetLayeredWindowAttributes = function (
     hwnd : HWND;         // handle to the layered window
     crKey : TColor;      // specifies the color key
     bAlpha : byte;       // value for the blend function
     dwFlags : DWORD      // action
     ): BOOL; stdcall;

	TRGBArray = array[0..32767] of TRGBTriple;
  PRGBArray = ^TRGBArray;

  function PNG_ExtractNineSlices(const sourceImage: TPNGObject; const gridRect: TRect): TPNGNineSlices; overload;
  function PNG_ExtractNineSlices(const sourceImage: TPNGObject): TPNGNineSlices; overload;
  procedure PNG_DrawNineSlices(const canvas: TCanvas; const nineSlices: TPNGNineSlices; const x, y, width, height: Integer);

  procedure DrawNineSlices_OLD(const canvas: TCanvas; const filePathPrefix: String; const x, y, targetWidth, targetHeight: Integer);
  function CreateRegion(Bmp: TBitmap): THandle;
  function ConstraintResize(sourceWidth, sourceHeight, targetWidth, targetHeight: Real; fitSmallerDimensions: Boolean):TSize;
  procedure SetTransparentForm(AHandle : THandle; AValue : byte = 0);

implementation



// By Thany: http://www.thany.org
function PNG_ExtractRectangularSlice(const sourceImage: TPNGObject; const sourceRect: TRect):TPNGObject;
var
   X, Y, OffsetX, OffsetY: Integer;
   Bitmap: TBitmap;
   BitmapLine: PRGBLine;
   AlphaLineA, AlphaLineB: pngimage.PByteArray;
   PNG: TPNGObject;

  //You might wanna define this function as inline if you have Delphi 2005 or higher.
  function ColorToTriple(Color: TColor): TRGBTriple;
  begin
  Color := ColorToRGB(Color);
  Result.rgbtBlue := Color shr 16 and $FF;
  Result.rgbtGreen := Color shr 8 and $FF;
  Result.rgbtRed := Color and $FF;
  end;

begin
  Bitmap := TBitmap.Create;
  try
    Bitmap.Width := sourceRect.Right - sourceRect.Left;
    Bitmap.Height := sourceRect.Bottom - sourceRect.Top;
    Bitmap.PixelFormat := pf24bit;

    OffsetX := sourceRect.Left;
    OffsetY := sourceRect.Top;

    //Copy the color information into a temporary bitmap. We can't use TPNGObject.Draw
    //here, because that would combine the color and alpha values.
    for Y := 0 to Bitmap.Height - 1
    do begin
       BitmapLine := Bitmap.Scanline[Y];
       for X := 0 to Bitmap.Width - 1
       do BitmapLine^[X] := ColorToTriple(sourceImage.Pixels[X + OffsetX, Y + OffsetY]);
       end;

    PNG := TPNGObject.Create;
    PNG.Assign(Bitmap);

    result := PNG;

    if sourceImage.Header.ColorType in [COLOR_GRAYSCALEALPHA, COLOR_RGBALPHA]
    then begin
         //Copy the alpha channel
         PNG.CreateAlpha;
         for Y := 0 to PNG.Height - 1
         do begin
            AlphaLineA := sourceImage.AlphaScanline[Y + OffsetY];
            AlphaLineB := PNG.AlphaScanline[Y];
            for X := 0 to PNG.Width - 1
            do AlphaLineB^[X] := AlphaLineA^[X + OffsetX];
            end;
         end;

  finally
    Bitmap.Free;
   end;
end;


function PNG_ExtractNineSlices(const sourceImage: TPNGObject): TPNGNineSlices; overload;
var rect: TRect;
begin
  rect.Left := Trunc(sourceImage.Width / 2);
  rect.Top := Trunc(sourceImage.Height / 2);
  rect.Right := rect.Left + 1;
  rect.Bottom := rect.Top + 1;

  result := PNG_ExtractNineSlices(sourceImage, rect);
end;


{*------------------------------------------------------------------------------
  This function returns an array of 9 TPNGObjects that must be drawn using
  DrawNineSlices. To see how 9 slice scaling works, see:
  http://www.creativepro.com/article/understanding-illustrator-s-9-slice-scaling

  @param sourceImage The image to slice
  @param gridRect The 9 slice scaling grid
  @return The 9 slices
-------------------------------------------------------------------------------}
function PNG_ExtractNineSlices(const sourceImage: TPNGObject; const gridRect: TRect): TPNGNineSlices; overload;
var r: TRect;
    imgWidth, imgHeight: Integer;
begin
  imgWidth := sourceImage.Width;
  imgHeight := sourceImage.Height;

  // ---------------------------------------------------------------------------
  // TOP
  // ---------------------------------------------------------------------------
  r.Left := 0;
  r.Top := 0;
  r.Right := gridRect.Left;
  r.Bottom := gridRect.Top;
  result[0] := PNG_ExtractRectangularSlice(sourceImage, r);

  r.Left := gridRect.Left;
  r.Top := 0;
  r.Right := gridRect.Right;
  r.Bottom := gridRect.Top;
  result[1] := PNG_ExtractRectangularSlice(sourceImage, r);

  r.Left := gridRect.Right;
  r.Top := 0;
  r.Right := imgWidth;
  r.Bottom := gridRect.Top;
  result[2] := PNG_ExtractRectangularSlice(sourceImage, r);

  // ---------------------------------------------------------------------------
  // MIDDLE
  // ---------------------------------------------------------------------------
  r.Left := 0;
  r.Top := gridRect.Top;
  r.Right := gridRect.Left;
  r.Bottom := gridRect.Bottom;
  result[3] := PNG_ExtractRectangularSlice(sourceImage, r);

  r.Left := gridRect.Left;
  r.Top := gridRect.Top;
  r.Right := gridRect.Right;
  r.Bottom := gridRect.Bottom;
  result[4] := PNG_ExtractRectangularSlice(sourceImage, r);

  r.Left := gridRect.Right;
  r.Top := gridRect.Top;
  r.Right := imgWidth;
  r.Bottom := gridRect.Bottom;
  result[5] := PNG_ExtractRectangularSlice(sourceImage, r);

  // ---------------------------------------------------------------------------
  // BOTTOM
  // ---------------------------------------------------------------------------
  r.Left := 0;
  r.Top := gridRect.Bottom;
  r.Right := gridRect.Left;
  r.Bottom := imgHeight;
  result[6] := PNG_ExtractRectangularSlice(sourceImage, r);

  r.Left := gridRect.Left;
  r.Top := gridRect.Bottom;
  r.Right := gridRect.Right;
  r.Bottom := imgHeight;
  result[7] := PNG_ExtractRectangularSlice(sourceImage, r);

  r.Left := gridRect.Right;
  r.Top := gridRect.Bottom;
  r.Right := imgWidth;
  r.Bottom := imgHeight;
  result[8] := PNG_ExtractRectangularSlice(sourceImage, r);
end;


{*------------------------------------------------------------------------------
  Draws a TPNGNineSlices object on the given canvas. See PNG_ExtractNineSlices

  @param canvas The canvas on which to draw the slices
  @param nineSlices The 9 slices
  @param x X offset of the drawn image
  @param y Y offset of the drawn image
  @param width The width of the drawn image
  @param height The height of the drawn image
-------------------------------------------------------------------------------}
procedure PNG_DrawNineSlices(const canvas: TCanvas; const nineSlices: TPNGNineSlices; const x, y, width, height: Integer);
var i: Integer;
    destRect: TRect;
    slice: TPNGObject;
begin
  for i := 0 to Length(nineSlices) - 1 do begin

    slice := nineSlices[i];

    case i of

      0: begin
        destRect.Left := x;
        destRect.Top := y;
        destRect.Right := x + slice.Width;
        destRect.Bottom := y + slice.Height;
      end;

      1: begin
        destRect.Left := destRect.Right;
        destRect.Right := x + width - nineSlices[2].Width;
      end;

      2: begin
        destRect.Left := destRect.Right;
        destRect.Right := x + width;
      end;

      3: begin
        destRect.Top := destRect.Bottom;
        destRect.Bottom := y + height - nineSlices[6].Height;
        destRect.Left := x;
        destRect.Right := x + slice.Width;

      end;

      4: begin
        destRect.Left := destRect.Right;
        destRect.Right := x + width - nineSlices[5].Width;
      end;

      5: begin
        destRect.Left := destRect.Right;
        destRect.Right := x + width;
      end;

      6: begin
        destRect.Top := destRect.Bottom;
        destRect.Bottom := y + height;
        destRect.Left := x;
        destRect.Right := x + slice.Width;
      end;

      7: begin
        destRect.Left := destRect.Right;
        destRect.Right := x + width - nineSlices[8].Width;
      end;

      8: begin
        destRect.Left := destRect.Right;
        destRect.Right := x + width;
      end;

    end; // case

    if destRect.Left >= destRect.Right then Continue;
    if destRect.Top >= destRect.Bottom then Continue;
    
    canvas.StretchDraw(destRect, slice);
    
  end; // for
  

end;



// By serge_perevoznyk@hotmail.com
// http://users.telenet.be/ws36637/transparent1.html 
procedure SetTransparentForm(AHandle : THandle; AValue : byte = 0);
var
 Info: TOSVersionInfo;
 SetLayeredWindowAttributes: TSetLayeredWindowAttributes;
begin
 //Check Windows version
 Info.dwOSVersionInfoSize := SizeOf(Info);
 GetVersionEx(Info);
 if (Info.dwPlatformId = VER_PLATFORM_WIN32_NT) and
 (Info.dwMajorVersion >= 5) then
   begin
     SetLayeredWindowAttributes := GetProcAddress(GetModulehandle(user32), 'SetLayeredWindowAttributes');
      if Assigned(SetLayeredWindowAttributes) then
       begin
        SetWindowLong(AHandle, GWL_EXSTYLE, GetWindowLong(AHandle, GWL_EXSTYLE) or WS_EX_LAYERED);
        //Make form transparent
        SetLayeredWindowAttributes(AHandle, 0, AValue, LWA_ALPHA);
      end;
   end;
end;


// By Gerard Oei
// http://www.delphi-central.com/BitmapShapedForm.aspx

function CreateRegion(Bmp: TBitmap): THandle;
var
  X, Y, StartX:Integer;
  Excl: THandle;
  Row: PRGBArray;
  TransparentColor: TRGBTriple;
begin
  // Change the format so we know how to compare 
  // the colors 
  Bmp.PixelFormat := pf24Bit;
    
  // Create a region of the whole bitmap 
  // later we will take the transparent   
  // bits away
  Result := CreateRectRGN(0, 0, Bmp.Width, Bmp.Height);

  // Loop down the bitmap   
  for Y := 0 to Bmp.Height - 1 do
  begin
    // Get the current row of pixels
    Row := Bmp.Scanline[Y];

    // If its the first get the transparent
    // color, it must be the top left pixel
    if Y = 0 then
    begin
      TransparentColor := Row[0];
    end;

    // Reset StartX (-1) to indicate we have
    // not found a transparent area yet
    StartX := -1;

    // Loop across the row
    for X := 0 to Bmp.Width do
    begin

      // Check for transparency by comparing the color
      if(X <> Bmp.Width) and
        (Row[X].rgbtRed = TransparentColor.rgbtRed) and
        (Row[X].rgbtGreen = TransparentColor.rgbtGreen) and
        (Row[X].rgbtBlue = TransparentColor.rgbtBlue) then
      begin
        // We have (X <> Bmp.Width) in the clause so that
        // when we go past the end of the row we we can
        // exclude the remaining transparent area (if any)
        // If its transparent and the previous wasn't
        // remember were the transparency started
        if StartX = -1 then
        begin
          StartX := X;
        end;
      end
      else
      begin
        // Its not transparent
        if StartX > -1 then
        begin
          // If previous pixels were transparent we
          // can now exclude the from the region
          Excl := CreateRectRGN(StartX, Y, X, Y + 1);
          try
            // Remove the exclusion from our original region
            CombineRGN(Result, Result, Excl, RGN_DIFF);

            // Reset StartX so we can start searching
            // for the next transparent area
            StartX := -1;
          finally
            DeleteObject(Excl);
          end;
     end;
      end;
    end;
  end; 
end;





function ConstraintResize(sourceWidth, sourceHeight, targetWidth, targetHeight: Real; fitSmallerDimensions: Boolean):TSize;
var targetRatio, sourceRatio: Real;
begin
  result.cx := Round(sourceWidth);
  result.cy := Round(sourceHeight);

  if (targetWidth = 0) or (targetHeight = 0) then Exit;
  if (sourceWidth = 0) or (sourceHeight = 0) then Exit;

	if (sourceWidth <= targetWidth) and (sourceHeight <= targetHeight) then begin

  	if fitSmallerDimensions then begin
    	targetRatio := targetWidth / targetHeight;
      sourceRatio := sourceWidth / sourceHeight;

      if sourceRatio < targetRatio then begin
      	result.cy := Round(targetHeight);
        result.cx := Round(result.cy * sourceRatio);
      end else begin
      	result.cx := Round(targetWidth);
        result.cy := Round(result.cx / sourceRatio);
      end;
    end;

  end else begin

    targetRatio := targetWidth / targetHeight;
    sourceRatio := sourceWidth / sourceHeight;

    if sourceRatio < targetRatio then begin
      result.cy := Round(targetHeight);
      result.cx := Round(result.cy * sourceRatio);
    end else begin
      result.cx := Round(targetWidth);
      result.cy := Round(result.cx / sourceRatio);
    end;
    
  end;

end;



procedure DrawNineSlices_OLD(const canvas: TCanvas; const filePathPrefix: String; const x, y, targetWidth, targetHeight: Integer);
var
	cornerSize: Integer;
	i, j: Byte;
  filePath: String;
  pngImage: TPNGObject;
  targetRect: TRect;
begin

	cornerSize := 0;
	targetRect.Left := 0;
  targetRect.Top := 0;

  for j := 0 to 2 do begin

  	for i := 0 to 2 do begin   

      filePath := filePathPrefix + IntToStr(i) + IntToStr(j) + '.png';
      pngImage := TPNGObject.Create();

      pngImage.LoadFromFile(filePath);

      if (cornerSize = 0) then cornerSize := pngImage.Width;

      case i of

      	0: begin
        	targetRect.Left := 0;
        	targetRect.Right := cornerSize;
        end;

        1: begin
          targetRect.Left := cornerSize;
        	targetRect.Right := targetWidth - cornerSize;
        end;

        2: begin
          targetRect.Left := targetWidth - cornerSize;
        	targetRect.Right := targetWidth;
        end;
        
      end;

      case j of

      	0: begin
        	targetRect.Top := 0;
        	targetRect.Bottom := cornerSize;
        end;

        1: begin
          targetRect.Top := cornerSize;
        	targetRect.Bottom := targetHeight - cornerSize;
        end;

        2: begin
          targetRect.Top := targetHeight - cornerSize;
        	targetRect.Bottom := targetHeight;
        end;
        
      end;

      targetRect.Left := targetRect.Left + x;
      targetRect.Right := targetRect.Right + x;
      targetRect.Top := targetRect.Top + y;
      targetRect.Bottom := targetRect.Bottom + y;

      canvas.StretchDraw(targetRect, pngImage);
      pngImage.Free();
      
    end;
  end;



end;


end.
