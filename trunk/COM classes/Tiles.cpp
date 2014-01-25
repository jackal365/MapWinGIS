/**************************************************************************************
 * File name: Tiles.h
 *
 * Project: MapWindow Open Source (MapWinGis ActiveX control) 
 * Description: implementation of CTiles
 *
 **************************************************************************************
 * The contents of this file are subject to the Mozilla Public License Version 1.1
 * (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at http://www.mozilla.org/mpl/ 
 * See the License for the specific language governing rights and limitations
 * under the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ************************************************************************************** 
 * Contributor(s): 
 * (Open source contributors should list themselves and their modifications here). */
 // lsu 21 aug 2011 - created the file

#include "stdafx.h"
#include "Tiles.h"
#include "SqliteCache.h"
#include "RamCache.h"
#include "map.h"
#include "DiskCache.h"

// ************************************************************
//		MarkUndrawn()
// ************************************************************
void CTiles::MarkUndrawn()
{
	for (unsigned int i = 0; i < m_tiles.size(); i++)
	{
		m_tiles[i]->m_drawn = false;
	}
}

// ************************************************************
//		UndrawnTilesExist()
// ************************************************************
// Returns true if at least one undrawn tile exists
bool CTiles::UndrawnTilesExist()
{
	for (unsigned int i = 0; i < m_tiles.size(); i++)
	{
		if (!m_tiles[i]->m_drawn)
			return true;
	}
	return false;
}

// ************************************************************
//		DrawnTilesExist()
// ************************************************************
// Returns true if at least one drawn tile exists
bool CTiles::DrawnTilesExist()
{
	for (unsigned int i = 0; i < m_tiles.size(); i++)
	{
		if (m_tiles[i]->m_drawn)
			return true;
	}
	return false;
}


#pragma region "ErrorHandling"
// ************************************************************
//		get_GlobalCallback()
// ************************************************************
STDMETHODIMP CTiles::get_GlobalCallback(ICallback **pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*pVal = m_globalCallback;
	if( m_globalCallback != NULL )
		m_globalCallback->AddRef();
	return S_OK;
}

// ************************************************************
//		put_GlobalCallback()
// ************************************************************
STDMETHODIMP CTiles::put_GlobalCallback(ICallback *newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	Utility::put_ComReference(newVal, (IDispatch**)&m_globalCallback);
	return S_OK;
}

// *****************************************************************
//	   get_ErrorMsg()
// *****************************************************************
STDMETHODIMP CTiles::get_ErrorMsg(long ErrorCode, BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	USES_CONVERSION;
	*pVal = A2BSTR(ErrorMsg(ErrorCode));
	return S_OK;
}

// ************************************************************
//		get_LastErrorCode()
// ************************************************************
STDMETHODIMP CTiles::get_LastErrorCode(long *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*pVal = m_lastErrorCode;
	m_lastErrorCode = tkNO_ERROR;
	return S_OK;
}

// **************************************************************
//		ErrorMessage()
// **************************************************************
void CTiles::ErrorMessage(long ErrorCode)
{
	m_lastErrorCode = ErrorCode;
	if( m_globalCallback != NULL) 
		m_globalCallback->Error(OLE2BSTR(m_key),A2BSTR(ErrorMsg(m_lastErrorCode)));
	return;
}

// ************************************************************
//		get/put_Key()
// ************************************************************
STDMETHODIMP CTiles::get_Key(BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	USES_CONVERSION;
	*pVal = OLE2BSTR(m_key);
	return S_OK;
}
STDMETHODIMP CTiles::put_Key(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	::SysFreeString(m_key);
	m_key = OLE2BSTR(newVal);
	return S_OK;
}

#pragma endregion

#pragma region Proxy
// *********************************************************
//	     AutodetectProxy()
// *********************************************************
STDMETHODIMP CTiles::AutodetectProxy(VARIANT_BOOL* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	*retVal = m_provider->AutodetectProxy();
	return S_OK;
}

// *********************************************************
//	     SetProxy()
// *********************************************************
STDMETHODIMP CTiles::SetProxy(BSTR address, int port, VARIANT_BOOL* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	USES_CONVERSION;
	*retVal = m_provider->SetProxy(OLE2A(address), port);
	return S_OK;
}

// *********************************************************
//	     get_Proxy()
// *********************************************************
STDMETHODIMP CTiles::get_Proxy(BSTR* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	USES_CONVERSION;
	CString s;
	s = m_provider->get_ProxyAddress();
	if (s.GetLength() == 0)
	{
		*retVal = A2BSTR("");
	}
	else
	{
		CString format = s + ":%d";
		short num = m_provider->get_ProxyPort();
		s.Format(format, num);
		*retVal = A2BSTR(s);
	}
	return S_OK;
}
#pragma endregion

#pragma region Load tiles
// ************************************************************
//		GetTileSize()
// ************************************************************
// Returns tiles size of the specified zoom level under current map scale
double CTiles::GetTileSize(PointLatLng& location, int zoom, double pixelPerDegree)
{
	CPoint point;
	m_provider->Projection->FromLatLngToXY(location, zoom, point);
	SizeLatLng size;
	m_provider->Projection->GetTileSizeLatLon(point, zoom, size);
	return (size.WidthLng + size.HeightLat) * pixelPerDegree / 2.0;
}

// ************************************************************
//		GetTileSize()
// ************************************************************
// Returns tiles size of the specified zoom level under currrne map scale
double CTiles::GetTileSizeByWidth(PointLatLng& location, int zoom, double pixelPerDegree)
{
	CPoint point;
	m_provider->Projection->FromLatLngToXY(location, zoom, point);
	SizeLatLng size;
	m_provider->Projection->GetTileSizeLatLon(point, zoom, size);
	return size.WidthLng * pixelPerDegree;
}

// ************************************************************
//		get_ClosestZoom()
// ************************************************************
int CTiles::ChooseZoom(double xMin, double xMax, double yMin, double yMax, 
					   double pixelPerDegree, bool limitByProvider, BaseProvider* provider)
{
	if (!provider)
		return 0;

	double lon = (xMax + xMin) / 2.0;
	double lat = (yMax + yMin) / 2.0;
	PointLatLng location(lat, lon);
	
	int bestZoom = provider->minZoom;
	for (int i = provider->minZoom; i <= (limitByProvider ? provider->maxZoom : 20); i++)
	{
		double tileSize = GetTileSizeByWidth(location, i, pixelPerDegree);
		if (tileSize > 256)
		{
			bestZoom = i;
		}
	}

	//Debug::WriteLine("best zoom: %d; tiles size: %f", bestZoom, GetTileSizeByWidth(location, bestZoom, pixelPerDegree));

	CSize s1, s2;
	provider->Projection->GetTileMatrixMinXY(bestZoom, s1);
	provider->Projection->GetTileMatrixMaxXY(bestZoom, s2);
	provider->minOfTiles.cx = s1.cx;
	provider->minOfTiles.cy = s1.cy;
	provider->maxOfTiles.cx = s2.cx;
	provider->maxOfTiles.cy = s2.cy;
	provider->zoom = bestZoom;

	return bestZoom;
}

// ************************************************************
//		ChooseZoom()
// ************************************************************
int CTiles::ChooseZoom(IExtents* ext, double pixelPerDegree, 
					   bool limitByProvider, BaseProvider* provider)
{
	double xMaxD, xMinD, yMaxD, yMinD, zMaxD, zMinD;
	ext->GetBounds(&xMinD, &yMinD, &zMinD, &xMaxD, &yMaxD, &zMaxD);
	//Debug::WriteLine("Extents changed: Lat = %f; Lng = %f; Lat2 = %f; Lng2 = %f", yMinD, xMinD, yMaxD, xMaxD);
	return this->ChooseZoom(xMinD, xMaxD, yMinD, yMaxD, pixelPerDegree, limitByProvider, provider);
}

// ************************************************************
//		get_CurrentZoom()
// ************************************************************
STDMETHODIMP CTiles::get_CurrentZoom(int* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (m_provider->mapView) {
		CMapView* map = (CMapView*)m_provider->mapView;
		IExtents* ext = map->GetGeographicExtents();
		if (ext) {
			*retVal = this->ChooseZoom(ext, map->GetPixelsPerDegree(), false, m_provider);
			ext->Release();
		}
	}
	return S_OK;
}

// ************************************************************
//		getRectangleXY()
// ************************************************************
void CTiles::getRectangleXY(double xMinD, double xMaxD, double yMinD, double yMaxD, int zoom, CRect &rect, BaseProvider* provider)
{
	// the number of tiles around
    if (provider)
	{
		CPoint p1, p2;

		provider->Projection->FromLatLngToXY(PointLatLng(yMaxD, xMinD), zoom, p1);
		provider->Projection->FromLatLngToXY(PointLatLng(yMinD, xMaxD), zoom, p2);

		rect.left = p1.x;
		rect.right = p2.x;
		rect.top = p1.y;
		rect.bottom = p2.y;
	}
}

bool CTiles::TilesAreInScreenBuffer(void* mapView)
{
	if (!m_visible || !m_provider) {
		return true;		// true because no tiles are actually needed for drawing, hence we need no tiles and yes we "have" them
	}

	int xMin, xMax, yMin, yMax, zoom;
	if (!GetTilesForMap(mapView, xMin, xMax, yMin, yMax, zoom))
		return true;
	
	for (int x = xMin; x <= xMax; x++)
	{
		for (int y = yMin; y <= yMax; y++)
		{
			bool found = false;
			for (size_t i = 0; i < m_tiles.size(); i++ )
			{
				if (m_tiles[i]->m_tileX == x && m_tiles[i]->m_tileY == y && m_tiles[i]->m_providerId == m_provider->Id)
				{
					found = true;
					break;
				}
			}
			if (!found)
				return false;
		}
	}
	return true;
}

// *********************************************************
//	     TilesAreInCache()
// *********************************************************
// checks whether all the tiles are present in cache
bool CTiles::TilesAreInCache(void* mapView, tkTileProvider providerId)
{
	
	BaseProvider* provider = providerId == ProviderNone ? m_provider : ((CTileProviders*)m_providers)->get_Provider(providerId);

	if (!m_visible || !provider) {
		return true;		// true because no tiles are actually needed for drawing, hence we need no tiles and yes we "have" them
	}

	int xMin, xMax, yMin, yMax, zoom;
	if (!GetTilesForMap(mapView, providerId, xMin, xMax, yMin, yMax, zoom))
		return true;
	
	for (int x = xMin; x <= xMax; x++)
	{
		for (int y = yMin; y <= yMax; y++)
		{
			bool found = false;
			for (size_t i = 0; i < m_tiles.size(); i++ )
			{
				if (m_tiles[i]->m_tileX == x && 
					m_tiles[i]->m_tileY == y && 
					m_tiles[i]->m_scale == zoom && 
					m_tiles[i]->m_providerId == provider->Id)
				{
					found = true;
					break;
				}
			}
			if (!found && m_useRamCache)
			{
				TileCore* tile = RamCache::get_Tile(provider->Id, zoom, x, y);
				if (tile)	found = true;
			}

			if (!found && m_useDiskCache)
			{
				TileCore* tile = SQLiteCache::get_Tile(provider, zoom, x, y);
				if (tile)	found = true;
			}

			if (!found)
				return false;
		}
	}
	return true;
}

bool CTiles::GetTilesForMap(void* mapView, int& xMin, int& xMax, int& yMin, int& yMax, int& zoom)
{
	return GetTilesForMap(mapView, -1, xMin, xMax, yMin, yMax, zoom);
}

// *********************************************************
//	     TilesAreInCache()
// *********************************************************
// returns list of tiles for current map extents
bool CTiles::GetTilesForMap(void* mapView, int providerId, int& xMin, int& xMax, int& yMin, int& yMax, int& zoom)
{
	BaseProvider* provider = providerId == -1 ? m_provider : ((CTileProviders*)m_providers)->get_Provider(providerId);
	if (!provider)
		return false;

	CMapView* map = (CMapView*)mapView;
	IExtents* ext = map->GetGeographicExtentsCore(true);
	if (!ext) {
		return false;
	}

	// schedule redraw
	if (!provider->mapView)
		provider->mapView = mapView;

	double xMaxD, xMinD, yMaxD, yMinD, zMaxD, zMinD;
	ext->GetBounds(&xMinD, &yMinD, &zMinD, &xMaxD, &yMaxD, &zMaxD);
	zoom = ChooseZoom(ext, map->GetPixelsPerDegree(), true, provider);
	ext->Release();

	// what tiles are needed?
	CRect rect;
	this->getRectangleXY(xMinD, xMaxD, yMinD, yMaxD, zoom, rect, provider);
	yMin = MIN(rect.top, rect.bottom);
	yMax = MAX(rect.top, rect.bottom);
	xMin = MIN(rect.left, rect.right);
	xMax = MAX(rect.left, rect.right);
	return true;
}

void CTiles::LoadTiles(void* mapView, bool isSnapshot, int providerId, CString key)
{
	BaseProvider* provider = ((CTileProviders*)m_providers)->get_Provider(providerId);

	if (!m_visible || !provider) {
		this->Clear();
		return;
	}
	
	int xMin, xMax, yMin, yMax, zoom;
	if (!GetTilesForMap(mapView, provider->Id, xMin, xMax, yMin, yMax, zoom))
	{
		this->Clear();
		return;
	}

	// schedule redraw
	this->m_tilesLoaded = false;
	for (size_t i = 0; i < m_tiles.size(); i++ )
	{
		m_tiles[i]->m_drawn = false;
		m_tiles[i]->m_inBuffer = false;
		m_tiles[i]->m_toDelete = true;
	}

	if (!provider->mapView)
		provider->mapView = mapView;
	
	int centX = (xMin + xMax) /2;
	int centY = (yMin + yMax) /2;

	SQLiteCache::Initialize();
	if (m_useDiskCache)
	{
		SQLiteCache::m_locked = true;	// caching will be stopped while loading tiles to avoid locking the database
	}
	
	std::vector<CTilePoint*> points;
	for (int x = xMin; x <= xMax; x++)
	{
		for (int y = yMin; y <= yMax; y++)
		{
			// first, check maybe the tile is already in the buffer
			for (size_t i = 0; i < m_tiles.size(); i++ )
			{
				TileCore* tile = m_tiles[i];
				if (tile->m_tileX == x && tile->m_tileY == y && tile->m_providerId == provider->Id)
				{
					tile->m_toDelete = false;	// it must not be deleted
					continue;
				}
			}
			
			if (m_useRamCache)
			{
				TileCore* tile = RamCache::get_Tile(provider->Id, zoom, x, y);
				if (tile)
				{
					this->AddTileNoCaching(tile);
					continue;
				}
			}

			if (m_useDiskCache)
			{
				TileCore* tile = SQLiteCache::get_Tile(provider, zoom, x, y);
				if (tile)
				{
					this->AddTileNoCaching(tile);
					continue;
				}
			}

			// if the tile isn't present in both caches, request it from the server
			if (m_useServer)
			{
				CTilePoint* pnt = new CTilePoint(x, y);
				pnt->dist = sqrt(pow((pnt->x - centX), 2.0) + pow((pnt->y - centY), 2.0));
				points.push_back(pnt);
			}
		}
	}

	// finally delete the unused tiles from the screen buffer
	std::vector<TileCore*>::iterator it = m_tiles.begin();
	while (it < m_tiles.end())
	{
		TileCore* tile = (*it);
		if (tile->m_toDelete)
		{
			tile->Release();
			it = m_tiles.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (m_useDiskCache)
	{
		SQLiteCache::m_locked = false;	// caching will be stopped while loading tiles to avoid locking the database and speed up things
	}
	m_tileLoader.RunCaching();

    if (points.size() > 0)
	{
		m_reloadCount++;
		m_tileLoader.Load(points, zoom, provider, (void*)this, isSnapshot, key);	// zoom can change in the process, so we use the calculated version
														// and not the one current for provider
		// releasing points
		for (size_t i = 0; i < points.size(); i++)
			delete points[i];
	}
}

// *********************************************************
//	     LoadTiles()
// *********************************************************
void CTiles::LoadTiles(void* mapView, bool isSnapshot, CString key)
{
	LoadTiles(mapView, isSnapshot, m_provider->Id, key);
}

// *********************************************************
//	     HandleOnTilesLoaded()
// *********************************************************
void CTiles::HandleOnTilesLoaded(bool isSnapshot, CString key)
{
	if ((CMapView*)m_provider->mapView != NULL)
	{
		LPCTSTR newStr = (LPCTSTR)key;
		this->AddRef();
		((CMapView*)m_provider->mapView)->FireTilesLoaded(this, NULL, isSnapshot, newStr);
	}
	//Fire_OnTilesLoaded(this, NULL, VARIANT_FALSE);
}

#pragma endregion

#pragma region Functions
// *********************************************************
//	     AddTileWithCaching()
// *********************************************************
void CTiles::AddTileWithCaching(TileCore* tile)
{
	this->AddTileNoCaching(tile);
	this->AddTileOnlyCaching(tile);
}

void CTiles::AddTileNoCaching(TileCore* tile)
{
	tile->m_inBuffer = true;
	tile->m_toDelete = false;
	tile->AddRef();
	m_tiles.push_back(tile);
	m_tilesLoaded = true;
}

void CTiles::AddTileOnlyCaching(TileCore* tile)
{
	if (m_doRamCaching) {
		RamCache::AddToCache(tile);
	}
	if (m_doDiskCaching) {
		m_tileLoader.ScheduleForCaching(tile);
	}
}

// *********************************************************
//	     Clear()
// *********************************************************
void CTiles::Clear()
{
	for (size_t i = 0; i < m_tiles.size(); i++)
	{
		m_tiles[i]->m_drawn = false;
		m_tiles[i]->m_inBuffer = false;
		m_tiles[i]->Release();
	}
	m_tiles.clear();
	m_tilesLoaded = false;
}
#pragma endregion

#pragma region Properties
// *********************************************************
//	     Provider
// *********************************************************
STDMETHODIMP CTiles::get_Provider(tkTileProvider* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CustomProvider* p = dynamic_cast<CustomProvider*>(m_provider);
	*pVal = p ? tkTileProvider::ProviderCustom : (tkTileProvider)m_provider->Id;
	return S_OK;
}
STDMETHODIMP CTiles::put_Provider(tkTileProvider newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (newVal < 0 || newVal >= tkTileProvider::ProviderCustom)
		return S_FALSE;

	if (m_provider->Id != newVal && newVal != tkTileProvider::ProviderCustom) 
	{
		m_provider = ((CTileProviders*)m_providers)->get_Provider((int)newVal);
		this->UpdateProjection();
	}
	return S_OK;
}

// *********************************************************
//	     get_ProviderName
// *********************************************************
STDMETHODIMP CTiles::get_ProviderName(BSTR* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	USES_CONVERSION;
	*retVal = m_provider ? A2BSTR(m_provider->Name) : A2BSTR("");
	return S_OK;
}

// *********************************************************
//	     GridLinesVisible
// *********************************************************
STDMETHODIMP CTiles::get_GridLinesVisible(VARIANT_BOOL* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_gridLinesVisible;
	return S_OK;
}
STDMETHODIMP CTiles::put_GridLinesVisible(VARIANT_BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_gridLinesVisible = newVal ? true: false;
	return S_OK;
}

// *********************************************************
//	     MinScaleToCache
// *********************************************************
STDMETHODIMP CTiles::get_MinScaleToCache(int* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_minScaleToCache;
	return S_OK;
}
STDMETHODIMP CTiles::put_MinScaleToCache(int newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_minScaleToCache = newVal;
	return S_OK;
}

// *********************************************************
//	     MaxScaleToCache
// *********************************************************
STDMETHODIMP CTiles::get_MaxScaleToCache(int* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_maxScaleToCache;
	return S_OK;
}
STDMETHODIMP CTiles::put_MaxScaleToCache(int newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_maxScaleToCache = newVal;
	return S_OK;
}

// *********************************************************
//	     Visible
// *********************************************************
STDMETHODIMP CTiles::get_Visible(VARIANT_BOOL* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_visible;
	return S_OK;
}

STDMETHODIMP CTiles::put_Visible(VARIANT_BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_visible = newVal != 0;
	return S_OK;
}

// *********************************************************
//	     CustomProviders
// *********************************************************
STDMETHODIMP CTiles::get_Providers(ITileProviders** retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_providers)
		m_providers->AddRef();
	*retVal = m_providers;
	return S_OK;
}

// *********************************************************
//	     DoCaching
// *********************************************************
STDMETHODIMP CTiles::get_DoCaching(tkCacheType type, VARIANT_BOOL* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			*pVal =  m_doRamCaching;
			break;
		case tkCacheType::Disk:
			*pVal = m_doDiskCaching;
			break;
		case tkCacheType::Both:
			*pVal = m_doRamCaching || m_doDiskCaching;
			break;
		default:
			*pVal = VARIANT_FALSE;
			break;
	}
	return S_OK;
}
STDMETHODIMP CTiles::put_DoCaching(tkCacheType type, VARIANT_BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			m_doRamCaching = newVal != 0;
			break;
		case tkCacheType::Disk:
			m_doDiskCaching = newVal != 0;
			break;
		case tkCacheType::Both:
			m_doDiskCaching = m_doRamCaching = newVal != 0;
			break;
	}
	return S_OK;
}

// *********************************************************
//	     UseCache
// *********************************************************
STDMETHODIMP CTiles::get_UseCache(tkCacheType type, VARIANT_BOOL* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			*pVal =  m_useRamCache;
			break;
		case tkCacheType::Disk:
			*pVal = m_useDiskCache;
			break;
		case tkCacheType::Both:
			*pVal = m_useRamCache || m_useDiskCache;
			break;
		default:
			*pVal = VARIANT_FALSE;
			break;
	}
	return S_OK;
}
STDMETHODIMP CTiles::put_UseCache(tkCacheType type, VARIANT_BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			m_useRamCache = newVal != 0;
			break;
		case tkCacheType::Disk:
			m_useDiskCache = newVal != 0;
			break;
		case tkCacheType::Both:
			m_useRamCache = m_useDiskCache = newVal != 0;
			break;
	}
	return S_OK;
}

// *********************************************************
//	     UseServer
// *********************************************************
STDMETHODIMP CTiles::get_UseServer(VARIANT_BOOL* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_useServer;
	return S_OK;
}
STDMETHODIMP CTiles::put_UseServer(VARIANT_BOOL newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	m_useServer = newVal != 0;
	return S_OK;
}

// *********************************************************
//	     get_DiskCacheFilename
// *********************************************************
STDMETHODIMP CTiles::get_DiskCacheFilename(BSTR* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	USES_CONVERSION;
	*retVal = A2BSTR(SQLiteCache::get_DbName());
	return S_OK;
}

// *********************************************************
//	     put_DiskCacheFilename
// *********************************************************
STDMETHODIMP CTiles::put_DiskCacheFilename(BSTR pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	USES_CONVERSION;
	CString s = OLE2A(pVal);
	SQLiteCache::set_DbName(s);
	return S_OK;
}

// *********************************************************
//	     MaxCacheSize
// *********************************************************
STDMETHODIMP CTiles::get_MaxCacheSize(tkCacheType type, double* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			*pVal =  RamCache::m_maxSize;
			break;
		case tkCacheType::Disk:
			*pVal = SQLiteCache::maxSizeDisk;
			break;
		default:
			*pVal = 0;
			break;
	}
	return S_OK;
}
STDMETHODIMP CTiles::put_MaxCacheSize(tkCacheType type, double newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			RamCache::m_maxSize = newVal;
			break;
		case tkCacheType::Disk:
			SQLiteCache::maxSizeDisk = newVal;
			break;
		case tkCacheType::Both:
			RamCache::m_maxSize = SQLiteCache::maxSizeDisk = newVal;
			break;
	}
	return S_OK;
}

// *********************************************************
//	     ClearCache()
// *********************************************************
STDMETHODIMP CTiles::ClearCache(tkCacheType type)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			RamCache::ClearByProvider(tkTileProvider::ProviderNone, 0, 100);
			break;
		case tkCacheType::Disk:
			SQLiteCache::Clear(tkTileProvider::ProviderNone, 0, 100);
			break;
		case tkCacheType::Both:
			RamCache::ClearAll(0, 100);
			SQLiteCache::Clear(tkTileProvider::ProviderNone, 0, 100);
			break;
	}
	return S_OK;
}

// *********************************************************
//	     ClearCache2()
// *********************************************************
STDMETHODIMP CTiles::ClearCache2(tkCacheType type, tkTileProvider provider, int fromScale, int toScale)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	switch(type)
	{
		case tkCacheType::RAM:
			RamCache::ClearByProvider(provider, fromScale, toScale);
			break;
		case tkCacheType::Disk:
			SQLiteCache::Clear(provider, fromScale, toScale);
			break;
		case tkCacheType::Both:
			SQLiteCache::Clear(provider, fromScale, toScale);
			RamCache::ClearByProvider(provider, fromScale, toScale);
			break;
	}
	return S_OK;
}

// *********************************************************
//	     get_CacheSize()
// *********************************************************
STDMETHODIMP CTiles::get_CacheSize(tkCacheType type, double* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*retVal = 0.0;
	switch(type)
	{
		case tkCacheType::RAM:
			*retVal = RamCache::get_MemorySize();
			break;
		case tkCacheType::Disk:
			*retVal = SQLiteCache::get_FileSize();
			break;
		case tkCacheType::Both:
			*retVal = 0.0;		// it hardly makes sense to show sum of both values or any of it separately
			break;
	}
	return S_OK;
}

// *********************************************************
//	     get_CacheSize3()
// *********************************************************
STDMETHODIMP CTiles::get_CacheSize2(tkCacheType type, tkTileProvider provider, int scale, double* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*retVal = 0.0;
	switch(type)
	{
		case tkCacheType::RAM:
			*retVal = RamCache::get_MemorySize(provider, scale);
			break;
		case tkCacheType::Disk:
			*retVal = SQLiteCache::get_FileSize(provider, scale);
			break;
		case tkCacheType::Both:
			*retVal = 0.0;	// it doesn't make sense to return any of the two
			break;
	}
	return S_OK;
}
#pragma endregion

#pragma region "Serialization"
// ********************************************************
//     Serialize()
// ********************************************************
STDMETHODIMP CTiles::Serialize(BSTR* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	USES_CONVERSION;

	CPLXMLNode* node = this->SerializeCore("TilesClass");
	if (node)
	{
		CString str = CPLSerializeXMLTree(node);	
		CPLDestroyXMLNode(node);
		*retVal = A2BSTR(str);
	}
	else
	{
		*retVal = A2BSTR("");
	}
	return S_OK;
}

// ********************************************************
//     SerializeCore()
// ********************************************************
CPLXMLNode* CTiles::SerializeCore(CString ElementName)
{
	USES_CONVERSION;
	CPLXMLNode* psTree = CPLCreateXMLNode( NULL, CXT_Element, ElementName);
	
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Visible", CPLString().Printf("%d", (int)m_visible));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "GridLinesVisible", CPLString().Printf("%d", (int)m_gridLinesVisible));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "Provider", CPLString().Printf("%d", (int)m_provider->Id));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "DoRamCaching", CPLString().Printf("%d", (int)m_doRamCaching));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "DoDiskCaching", CPLString().Printf("%d", (int)m_doDiskCaching));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "UseRamCache", CPLString().Printf("%d", (int)m_useRamCache));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "UseDiskCache", CPLString().Printf("%d", (int)m_useDiskCache));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "UseServer", CPLString().Printf("%d", (int)m_useServer));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "MaxRamCacheSize", CPLString().Printf("%f", RamCache::m_maxSize));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "MaxDiskCacheSize", CPLString().Printf("%f", SQLiteCache::maxSizeDisk));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "MinScaleToCache", CPLString().Printf("%d", m_minScaleToCache));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "MaxScaleToCache", CPLString().Printf("%d", m_maxScaleToCache));
	Utility::CPLCreateXMLAttributeAndValue(psTree, "DiskCacheFilename", SQLiteCache::get_DbName());
	return psTree;
}

// ********************************************************
//     Deserialize()
// ********************************************************
STDMETHODIMP CTiles::Deserialize(BSTR newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	USES_CONVERSION;

	CString s = OLE2CA(newVal);
	CPLXMLNode* node = CPLParseXMLString(s.GetString());
	if (node)
	{
		CPLXMLNode* nodeTiles = CPLGetXMLNode(node, "=TilesClass");
		if (nodeTiles)
		{
			DeserializeCore(nodeTiles);
		}
		CPLDestroyXMLNode(node);
	}
	return S_OK;
}

void setBoolean(CPLXMLNode *node, CString name, bool& value)
{
	CString s = CPLGetXMLValue( node, name, NULL );
	if (s != "") value = atoi( s ) == 0? false : true;
}

void setInteger(CPLXMLNode *node, CString name, int& value)
{
	CString s = CPLGetXMLValue( node, name, NULL );
	if (s != "") value = atoi( s );
}

void setDouble(CPLXMLNode *node, CString name, double& value)
{
	CString s = CPLGetXMLValue( node, name, NULL );
	if (s != "") value = Utility::atof_custom( s );
}

// ********************************************************
//     DeserializeCore()
// ********************************************************
bool CTiles::DeserializeCore(CPLXMLNode* node)
{
	if (!node)
		return false;

	this->setDefaults();

	setBoolean(node, "Visible", m_visible);
	setBoolean(node, "GridLinesVisible", m_gridLinesVisible);
	setBoolean(node, "DoRamCaching", m_doRamCaching);
	setBoolean(node, "DoDiskCaching", m_doDiskCaching);
	setBoolean(node, "UseRamCache", m_useRamCache);
	setBoolean(node, "UseDiskCache", m_useDiskCache);
	setBoolean(node, "UseServer", m_useServer);

	CString s = CPLGetXMLValue( node, "Provider", NULL );
	if (s != "") this->put_ProviderId(atoi( s ));

	setDouble(node, "MaxRamCacheSize", RamCache::m_maxSize);
	setDouble(node, "MaxDiskCacheSize", SQLiteCache::maxSizeDisk);
	setInteger(node, "MinScaleToCache", m_minScaleToCache);
	setInteger(node, "MaxScaleToCache", m_maxScaleToCache);
	
	s = CPLGetXMLValue( node, "DiskCacheFilename", NULL );
	if (s != "") SQLiteCache::set_DbName(s);
	return true;
}
#pragma endregion

// *********************************************************
//	     CustomProviderId
// *********************************************************
STDMETHODIMP CTiles::get_ProviderId(int* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*retVal = m_provider->Id;
	return S_OK;
}

STDMETHODIMP CTiles::put_ProviderId(int providerId)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	BaseProvider* provider = ((CTileProviders*)m_providers)->get_Provider(providerId);
	if (provider) {
		m_provider = provider;
		this->UpdateProjection();
	}
	return S_OK;
}

// *********************************************************
//	     GetTilesIndices
// *********************************************************
STDMETHODIMP CTiles::GetTilesIndices(IExtents* boundsDegrees, int zoom, int providerId, IExtents** retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!boundsDegrees)
	{
		ErrorMessage(tkUNEXPECTED_NULL_PARAMETER);
		return S_FALSE;
	}
	
	double xMin, xMax, yMin, yMax, zMin, zMax;
	boundsDegrees->GetBounds(&xMin, &yMin, &zMin, &xMax, &yMax, &zMax);
	
	BaseProvider* provider = ((CTileProviders*)m_providers)->get_Provider(providerId);
	if (provider)
	{
		CPoint p1;
		provider->Projection->FromLatLngToXY(PointLatLng(yMax, xMin), zoom, p1);

		CPoint p2;
		provider->Projection->FromLatLngToXY(PointLatLng(yMin, xMax), zoom, p2);

		IExtents* ext = NULL;
		CoCreateInstance( CLSID_Extents, NULL, CLSCTX_INPROC_SERVER, IID_IExtents, (void**)&ext);
		ext->SetBounds(p1.x, p1.y, 0.0, p2.x, p2.y, 0.0);
		*retVal = ext;
	}
	else {
		*retVal = NULL;
	}
	return S_OK;
}

// *********************************************************
//	     Prefetch
// *********************************************************
STDMETHODIMP CTiles::Prefetch(double minLat, double maxLat, double minLng, double maxLng, int zoom, int providerId, 
							  IStopExecution* stop, LONG* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	BaseProvider* p = ((CTileProviders*)m_providers)->get_Provider(providerId);
	if (!p)
	{
		ErrorMessage(tkINVALID_PROVIDER_ID);
		return S_FALSE;
	}
	else
	{
		CPoint p1;
		p->Projection->FromLatLngToXY(PointLatLng(minLat, minLng), zoom, p1);

		CPoint p2;
		p->Projection->FromLatLngToXY(PointLatLng(maxLat, maxLng), zoom, p2);

		this->Prefetch2(p1.x, p2.x, MIN(p1.y, p2.y) , MAX(p1.y, p2.y), zoom, providerId, stop, retVal);
		return S_OK;
	}
}

// *********************************************************
//	     Prefetch2
// *********************************************************
STDMETHODIMP CTiles::Prefetch2(int minX, int maxX, int minY, int maxY, int zoom, int providerId, IStopExecution* stop, LONG* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*retVal = PrefetchCore(minX, maxX, minY, maxY, zoom, providerId, A2BSTR(""), A2BSTR(""), stop);
	return S_OK;
}

// *********************************************************
//	     PrefetchToFolder()
// *********************************************************
// Writes tiles to the specified folder
STDMETHODIMP CTiles::PrefetchToFolder(IExtents* ext, int zoom, int providerId, BSTR savePath, BSTR fileExt, IStopExecution* stop, LONG* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*retVal = 0;
	
	USES_CONVERSION;
	CString path = OLE2A(savePath);
	if (!Utility::dirExists(path))
	{
		ErrorMessage(tkFOLDER_NOT_EXISTS);
		return S_FALSE;
	}
	
	BaseProvider* p = ((CTileProviders*)m_providers)->get_Provider(providerId);
	if (!p)
	{
		ErrorMessage(tkINVALID_PROVIDER_ID);
		return S_FALSE;
	}
	else
	{
		double xMin, xMax, yMin, yMax, zMin, zMax;
		ext->GetBounds(&xMin, &yMin, &zMin, &xMax, &yMax, &zMax);

		CPoint p1;
		p->Projection->FromLatLngToXY(PointLatLng(yMin, xMin), zoom, p1);

		CPoint p2;
		p->Projection->FromLatLngToXY(PointLatLng(yMax, xMax), zoom, p2);

		*retVal = this->PrefetchCore(p1.x, p2.x, MIN(p1.y, p2.y) , MAX(p1.y, p2.y), zoom, providerId, savePath, fileExt, stop);
	}
	return S_OK;
}

// *********************************************************
//	     PrefetchToFolder()
// *********************************************************
long CTiles::PrefetchCore(int minX, int maxX, int minY, int maxY, int zoom, int providerId, 
								  BSTR savePath, BSTR fileExt, IStopExecution* stop)
{
	BaseProvider* provider = ((CTileProviders*)m_providers)->get_Provider(providerId);
	if (!provider)
	{
		ErrorMessage(tkINVALID_PROVIDER_ID);
		return S_FALSE;
	}
	else if (provider->maxZoom < zoom) 
	{
		ErrorMessage(tkINVALID_ZOOM_LEVEL);
		return S_FALSE;
	}
	else
	{
		CSize size1, size2;
		provider->Projection->GetTileMatrixMinXY(zoom, size1);
		provider->Projection->GetTileMatrixMaxXY(zoom, size2);

		minX = (int)BaseProjection::Clip(minX, size1.cx, size2.cx);
		maxX = (int)BaseProjection::Clip(maxX, size1.cy, size2.cy);
		minY = (int)BaseProjection::Clip(minY, size1.cx, size2.cx);
		maxY = (int)BaseProjection::Clip(maxY, size1.cy, size2.cy);
		
		int centX = (maxX + minX)/2;
		int centY = (maxY + minY)/2;

		USES_CONVERSION;
		CString path = OLE2A(savePath);
		if (path.GetLength() > 0 && path.GetAt(path.GetLength() - 1) != '\\')
		{
			path += "\\";
		}
		CacheType type = path.GetLength() > 0 ? CacheType::DiskCache : CacheType::SqliteCache;

		std::vector<CTilePoint*> points;
		for (int x = minX; x <= maxX; x++)
		{
			for (int y = minY; y <= maxY; y++)
			{
				if ((type == CacheType::SqliteCache && !SQLiteCache::get_Exists(provider, zoom, x, y)) 
					|| type == CacheType::DiskCache)
				{
					CTilePoint* pnt = new CTilePoint(x, y);
					pnt->dist = sqrt(pow((pnt->x - centX), 2.0) + pow((pnt->y - centY), 2.0));
					points.push_back(pnt);
				}
			}
		}

		if (points.size() > 0)
		{
			loader.doCacheSqlite = type == CacheType::SqliteCache;
			loader.doCacheDisk = type == CacheType::DiskCache;
			
			if (type == CacheType::SqliteCache)
			{
				SQLiteCache::Initialize();
			}
			else 
			{
				CString ext = OLE2A(fileExt);
				DiskCache::ext = ext;
				DiskCache::rootPath = path;
				DiskCache::encoder = "image/png";	// default
				
				if (ext.GetLength() >= 4)
				{
					CString s = ext.Mid(0, 4).MakeLower(); // try to guess it from input
					if (s == ".png")
					{
						DiskCache::encoder = "image/png";
					}
					else if (s == ".jpg")
					{
						DiskCache::encoder = "image/jpeg";
					}
				}
				DiskCache::CreateFolders(zoom, points);
			}
			loader.m_stopCallback = stop;

			if (m_globalCallback)
			{
				loader.m_callback = this->m_globalCallback;
			}
			
			// actual call to do the job
			loader.Load(points, zoom, provider, (void*)this, false, "", true);

			for (size_t i = 0; i < points.size(); i++)
			{
				delete points[i];
			}
			return points.size();
		}
	}
	return 0;
}

// *********************************************************
//	     get_DiskCacheCount
// *********************************************************
STDMETHODIMP CTiles::get_DiskCacheCount(int provider, int zoom, int xMin, int xMax, int yMin, int yMax, LONG* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*retVal = SQLiteCache::get_TileCount(provider, zoom, xMin, xMax, yMin, yMax);
	return S_OK;
}

// *********************************************************
//	     CheckConnection
// *********************************************************
STDMETHODIMP CTiles::CheckConnection(BSTR url, VARIANT_BOOL* retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_provider != NULL)
	{
		USES_CONVERSION;
		*retVal = m_provider->CheckConnection(OLE2A(url)) ? VARIANT_TRUE: VARIANT_FALSE;
	}
	else {
		*retVal = VARIANT_FALSE;
	}
	return S_OK;
}

// *********************************************************
//	     get_TileBounds
// *********************************************************
STDMETHODIMP CTiles::GetTileBounds(int provider, int zoom, int tileX, int tileY, IExtents** retVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	BaseProvider* prov = ((CTileProviders*)m_providers)->get_Provider(provider);
	if (!prov)
	{
		ErrorMessage(tkINVALID_PROVIDER_ID);
		return S_FALSE;
	}
	else
	{
		CSize size;
		prov->Projection->GetTileMatrixSizeXY(zoom, size);
		if (tileX < 0 || tileX > size.cx - 1 || tileY < 0 || tileY > size.cy - 1)
		{
			ErrorMessage(tkINDEX_OUT_OF_BOUNDS);
		}
		else 
		{
			CPoint pnt1(tileX, tileY);
			CPoint pnt2(tileX + 1, tileY + 1);
			PointLatLng p1, p2;
			prov->Projection->FromXYToLatLng(pnt1, zoom, p1);
			prov->Projection->FromXYToLatLng(pnt2, zoom, p2);
			IExtents* ext;
			CoCreateInstance( CLSID_Extents, NULL, CLSCTX_INPROC_SERVER, IID_IExtents, (void**)&ext);
			ext->SetBounds(p1.Lng, p1.Lat, 0.0, p2.Lng, p2.Lat, 0.0);
			*retVal = ext;
		}
	}
	return S_OK;
}

// ************************************************************
//		Zoom()
// ************************************************************
// Sets the closest discrete zoom; not finished; probably should be updated 
// using code from CMapView::ZoomToTileScale
void CTiles::Zoom(bool out)
{
	CMapView* map = (CMapView*)m_provider->mapView;
	if (map) {
		IExtents* ext = map->GetGeographicExtents();
		if (ext) {
			double xMin, xMax, yMin, yMax, zMin, zMax;
			ext->GetBounds(&xMin, &yMin, &zMin, &xMax, &yMax, &zMax);
			double lon = (xMax + xMin) / 2.0;
			double lat = (yMax + yMin) / 2.0;
			PointLatLng loc(lat, lon);
			
			double tileSize = (double)m_provider->Projection->tileSize.cx;
			double size = GetTileSize(loc, m_provider->zoom, map->GetPixelsPerDegree());
			if ((size > tileSize && out) ||
				size < tileSize && !out)
			{
				// remain at the same oom level; do nothing
			}
			else if (size > tileSize && !out) 
			{
				//size = GetTileSize(loc, m_provider->zoom + 1, map->GetPixelsPerDegree());
				// next level
			}
			else if (size < tileSize && out)
			{
				//size = GetTileSize(loc, m_provider->zoom - 1, map->GetPixelsPerDegree());
				// previous level
			}

			double ratio = size/tileSize;
			
			double w = (xMax - xMin) * ratio;
			double h =(yMax - yMin) * ratio;
			ext->SetBounds(lon - w/2.0, lat - h/2.0, 0.0, lon + w/2.0, lat + h/2.0, 0.0);
			map->SetGeographicExtents(ext);
		}
	}
}

// ************************************************************
//		ProjectionBounds
// ************************************************************
bool CTiles::ProjectionBounds(IGeoProjection* wgsProjection, bool doTransformtation, Extent& retVal)
{
	if (!wgsProjection) {
		return false;
	}
	if (m_projExtentsNeedUpdate)
	{
		BaseProjection* pr = this->m_provider->Projection;
		double left =  pr->MinLongitude;
		double right = pr->MaxLongitude;
		double top = pr->MaxLatitude;
		double bottom = pr->MinLatitude;
		
		if (doTransformtation)
		{
			VARIANT_BOOL vb;
			wgsProjection->Transform(&left, &top, &vb);
			if (!vb) {
				Debug::WriteLine("Failed to project: x = %f; y = %f", left, top);
				return false;
			}
			wgsProjection->Transform(&right, &bottom, &vb);
			if (!vb) {
				Debug::WriteLine("Failed to project: x = %f; y = %f", bottom, right);
				return false;
			}
			//Debug::WriteLine("Projected world bounds: left = %f; right = %f; bottom = %f; top = %f", left, right, bottom, top);
		}
		
		m_projExtents.left = left;
		m_projExtents.right = right;
		m_projExtents.top = top;
		m_projExtents.bottom = bottom;
	}
	retVal.left = m_projExtents.left;
	retVal.right = m_projExtents.right;
	retVal.top = m_projExtents.top;
	retVal.bottom = m_projExtents.bottom;
	return true;
}
