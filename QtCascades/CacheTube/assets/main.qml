import bb.cascades 1.0
import bb.system 1.0
import YTVideoManagement 1.0

TabbedPane {
    id: tabbedPane

    onActiveTabChanged: {
        if (activeTab === helpTab) {
            helpWebView.url = qsTr("local:///assets/doc/help.html");
        }
    }
    
    Tab {
        id:          youTubeTab
        title:       qsTr("YouTube")
        imageSource: "images/youtube.png"

        Page {
            actions: [
                ActionItem {
                    title:               qsTr("Back")
                    imageSource:         "images/back.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        youTubeWebView.goBack();
                    }
                },
                ActionItem {
                    title:               qsTr("Home")
                    imageSource:         "images/home.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        youTubeWebView.url = "http://m.youtube.com/";
                    }
                },
                ActionItem {
                    title:               qsTr("Cache")
                    imageSource:         "images/cache.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        var video_id = YTVideoManager.getVideoId(youTubeWebView.url);
                        
                        if (video_id !== "") {
                            if (YTVideoManager.addTask(video_id)) {
                                cachingStartedToast.show();
                            } else {
                                cachingFailedToast.show();
                            }
                        }
                    }
                    
                    attachedObjects: [
                        SystemToast {
                            id:   cachingStartedToast
                            body: qsTr("Video caching started") 
                        },
                        SystemToast {
                            id:   cachingFailedToast
                            body: qsTr("Could not cache this video. May be it is already cached?")
                        }
                    ]
                },
                ActionItem {
                    title:               qsTr("Reload")
                    imageSource:         "images/reload.png"
                    ActionBar.placement: ActionBarPlacement.InOverflow

                    onTriggered: {
                        youTubeWebView.reload();
                    }
                }
            ]

            Container {
                background: Color.White
                
                layout: DockLayout {
                }

                ScrollView {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    
                    scrollViewProperties {
                        scrollMode:         ScrollMode.Both
                        pinchToZoomEnabled: true
                        minContentScale:    1.0
                        maxContentScale:    4.0
                    }

                    WebView {
                        id:  youTubeWebView
                        url: "http://m.youtube.com/"
                    }
                }
                
                ProgressIndicator {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Bottom
                    visible:             youTubeWebView.loading
                    fromValue:           0
                    toValue:             100
                    value:               youTubeWebView.loadProgress
                }
            }
        }
    }
    
    Tab {
        title:       qsTr("Cached Video")
        imageSource: "images/cache.png"

        Page {
            titleBar: TitleBar {
                title: qsTr("Cached Video")
            }

            Container {
                background: Color.White

                layout: DockLayout {
                }

                Container {
                    horizontalAlignment: HorizontalAlignment.Center
                    verticalAlignment:   VerticalAlignment.Center
                    background:          Color.Transparent
                    visible:             cacheListViewDataModel.itemsCount <= 0
                    
                    layout: StackLayout {
                    }
                    
                    ImageView {
                        horizontalAlignment: HorizontalAlignment.Center
                        imageSource:         "images/cache_empty.png"
                    }
                    
                    Label {
                        horizontalAlignment: HorizontalAlignment.Center
                        textStyle.color:     Color.Black
                        textStyle.fontSize:  FontSize.XLarge
                        text:                qsTr("No cached video")
                    }
                }

                ListView {
                    horizontalAlignment: HorizontalAlignment.Fill
                    verticalAlignment:   VerticalAlignment.Fill
                    visible:             cacheListViewDataModel.itemsCount > 0

                    property variant ytVideoManager: YTVideoManager 

                    dataModel: YTArrayDataModel {
                        id:           cacheListViewDataModel
                        videoManager: YTVideoManager
                    }
                    
                    function navigateToWebPage(url) {
                        youTubeWebView.url = url;

                        tabbedPane.activeTab = youTubeTab;
                    } 
                    
                    listItemComponents: [
                        ListItemComponent {
                            type: ""
                            
                            Container {
                                id:           itemRoot
                                background:   Color.Transparent 
                                leftPadding:  12
                                rightPadding: 12

                                property int    itemState:    ListItemData.state
                                property int    itemSize:     ListItemData.size
                                property int    itemDone:     ListItemData.done
                                property string itemVideoId:  ListItemData.videoId
                                property string itemMimeType: ListItemData.mimeType
                                property string itemTitle:    ListItemData.title
                                property string itemErrorMsg: ListItemData.errorMsg

                                layout: StackLayout {
                                    orientation: LayoutOrientation.TopToBottom
                                }

                                contextActions: [
                                    ActionSet {
                                        ActionItem {
                                            title:       qsTr("Delete")
                                            imageSource: "images/delete.png"
                                            
                                            onTriggered: {
                                                itemRoot.ListItem.view.ytVideoManager.delTask(itemRoot.itemVideoId);
                                            }
                                        }

                                        ActionItem {
                                            title:       qsTr("Pause/Resume")
                                            imageSource: "images/pause_resume.png"
                                            enabled:     itemRoot.itemState !== YTDownloadState.StateCompleted

                                            onTriggered: {
                                                if (itemRoot.itemState === YTDownloadState.StatePaused) {
                                                    itemRoot.ListItem.view.ytVideoManager.resumeTask(itemRoot.itemVideoId);
                                                } else {
                                                    itemRoot.ListItem.view.ytVideoManager.pauseTask(itemRoot.itemVideoId);
                                                }
                                            }
                                        }

                                        ActionItem {
                                            title:       qsTr("Play Video")
                                            imageSource: "images/play_video.png"
                                            enabled:     itemRoot.itemState === YTDownloadState.StateCompleted

                                            onTriggered: {
                                                playerNavigationPane.push(playerPageDefinition.createObject());

                                                playerSheet.open();
                                            }
                                            
                                            attachedObjects: [
                                                Sheet {
                                                    id:          playerSheet
                                                    peekEnabled: false

                                                    onOpened: {
                                                        var page = playerNavigationPane.top;
                                                        
                                                        if (page.objectName === "playerPage") {
                                                            page.playVideo(itemRoot.ListItem.view.ytVideoManager.getTaskVideoURI(itemRoot.itemVideoId), itemRoot.itemTitle);
                                                        }
                                                    }

                                                    NavigationPane {
                                                        id:          playerNavigationPane
                                                        peekEnabled: false

                                                        onTopChanged: {
                                                            if (page.objectName === "playerEmptyPage") {
                                                                playerSheet.close();
                                                            }
                                                        }
                                                        
                                                        onPopTransitionEnded: {
                                                            page.destroy();
                                                        }
                                                        
                                                        Page {
                                                            objectName: "playerEmptyPage"
                                                            
                                                            Container {
                                                                background: Color.White
                                                            }
                                                        }
                                                    }
                                                    
                                                    attachedObjects: [
                                                        ComponentDefinition {
                                                            id:     playerPageDefinition
                                                            source: "playerPage.qml"
                                                        }
                                                    ]
                                                }
                                            ]
                                        }

                                        ActionItem {
                                            title:       qsTr("Open YouTube Page")
                                            imageSource: "images/youtube.png"

                                            onTriggered: {
                                                itemRoot.ListItem.view.navigateToWebPage(itemRoot.ListItem.view.ytVideoManager.getTaskWebURL(itemRoot.itemVideoId));
                                            }
                                        }
                                    }
                                ]

                                Label {
                                    multiline:            true
                                    textFormat:           TextFormat.Plain
                                    textStyle.color:      Color.Black
                                    textStyle.fontWeight: FontWeight.Bold
                                    textStyle.fontSize:   FontSize.Medium
                                    text:                 itemRoot.itemTitle
                                }

                                ProgressIndicator {
                                    visible:   itemRoot.itemState === YTDownloadState.StateActive
                                    fromValue: 0
                                    toValue:   itemRoot.itemSize
                                    value:     itemRoot.itemDone
                                }

                                Label {
                                    visible:             itemRoot.itemState !== YTDownloadState.StateActive
                                    multiline:           true
                                    textStyle.color:     itemRoot.itemState === YTDownloadState.StateError ? Color.Red : Color.Grey
                                    textStyle.fontStyle: FontStyle.Italic
                                    textStyle.fontSize:  FontSize.Small
                                    text:                itemRoot.itemState === YTDownloadState.StateCompleted ? (itemRoot.itemSize / 1048576).toFixed(2) + " MiB" :
                                                        (itemRoot.itemState === YTDownloadState.StateError     ? itemRoot.itemErrorMsg                             :
                                                        (itemRoot.itemState === YTDownloadState.StateQueued    ? qsTr("QUEUED")                                    :
                                                        (itemRoot.itemState === YTDownloadState.StatePaused    ? qsTr("PAUSED")                                    : "")))
                                }

                                Divider {
                                }
                            }
                        }
                    ]
                }
            }
        }
    }
    
    Tab {
        title:       qsTr("Settings")
        imageSource: "images/settings.png"

        Page {
            titleBar: TitleBar {
                title: qsTr("Settings")
            }

            Container {
                background: Color.White

                ScrollView {
                    scrollViewProperties {
                        scrollMode: ScrollMode.Vertical
                    }

                    Container {
                        background:   Color.Transparent
                        leftPadding:  12
                        rightPadding: 12

                        layout: StackLayout {
                        }

                        Label {
                            textStyle.color:    Color.Black
                            textStyle.fontSize: FontSize.Large
                            text:               qsTr("Preferred Video Format:")
                        }

                        Divider {
                        }

                        RadioGroup {
                            onCreationCompleted: {
                                var format = AppSettings.preferredVideoFormat;
                                
                                if (format === 22) {
                                    selectedIndex = 0;
                                } else if (format === 18) {
                                    selectedIndex = 1;
                                } else {
                                    selectedIndex = 0;
                                }
                            }
                            
                            onSelectedIndexChanged: {
                                if (selectedIndex === 0) {
                                    AppSettings.preferredVideoFormat = 22;

                                    YTVideoManager.setPreferredVideoFormat(22);
                                } else if (selectedIndex === 1) {
                                    AppSettings.preferredVideoFormat = 18;

                                    YTVideoManager.setPreferredVideoFormat(18);
                                }
                            }
                            
                            Option {
                                text: qsTr("720p H.264 MP4")
                            }

                            Option {
                                text: qsTr("360p H.264 MP4")
                            }
                        }
                        
                        Divider {
                        }
                    }
                }
            }
        }
    }
    
    Tab {
        id:          helpTab
        title:       qsTr("Help")
        imageSource: "images/help.png" 
        
        Page {
            actions: [
                ActionItem {
                    title:               qsTr("Review App")
                    imageSource:         "images/review.png"
                    ActionBar.placement: ActionBarPlacement.OnBar

                    onTriggered: {
                        appWorldInvocation.trigger("bb.action.OPEN");
                    }

                    attachedObjects: [
                        Invocation {
                            id: appWorldInvocation

                            query: InvokeQuery {
                                mimeType: "application/x-bb-appworld"
                                uri:      "appworld://content/42093887"
                            }
                        }
                    ]
                }
            ]

            ScrollView {
                scrollViewProperties {
                    scrollMode:         ScrollMode.Both
                    pinchToZoomEnabled: true
                    minContentScale:    1.0
                    maxContentScale:    4.0
                }

                WebView {
                    id:  helpWebView
                    url: qsTr("local:///assets/doc/help.html")
                }
            }
        }
    }
}
