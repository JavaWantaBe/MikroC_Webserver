

function addEvent(object, evName, fnName, cap) {
   if (object.attachEvent)
       object.attachEvent("on" + evName, fnName);
   else if (object.addEventListener)
       object.addEventListener(evName, fnName, cap);
}

// XMLHttpRequest Constructor
if (typeof XMLHttpRequest == "undefined") {
   XMLHttpRequest = function() {

      // Array of MSXML PIDs
      var pids = ["Msxml2.XMLHTTP.6.0",
                  "Msxml2.XMLHTTP.3.0",
                  "Msxml2.XMLHTTP",
                  "Microsoft.XMLHTTP"];

      // Test each PID
      for (var i = 0; i < pids.length; i++) {
         try {
            return new ActiveXObject(pids[i]);
         } catch (e) {}
      }

      // Halt if unable to create request object
      throw new Error("Unable to create request object");
   }
}



addEvent(window, "load", setuppage, false);

function setuppage() {
   var archiveDIV = document.getElementById("reviewarchives");

   // Request object for the authorlist JSON file
   var reqAuthors = new XMLHttpRequest();
   reqAuthors.open("GET", "author.txt");
   reqAuthors.setRequestHeader("Cache-Control", "no-cache");
   reqAuthors.send(null);

   reqAuthors.onreadystatechange = function() {
      // Process the data when the response is
      // completed without error
      if (this.readyState == 4) {
         if (this.status == 200) {
            // Retrieve the list of authors

            var json = eval("(" + this.responseText +")");

            var authorSelect = document.createElement("select");
            authorSelect.size = 5;
            var defaultOption = document.createElement("option");
            defaultOption.value="";
            defaultOption.innerHTML="Select an author to review";
            authorSelect.appendChild(defaultOption);

            for (var i = 0; i < json.authorlist.length; i++) {

               var newOption = document.createElement("option");
               newOption.value = json.authorlist[i].initials;
               newOption.innerHTML = json.authorlist[i].name;
               authorSelect.appendChild(newOption);
           }   

         authorSelect.onchange = function() {
            var selIndex = this.selectedIndex;
            var selInitials = this.options[selIndex].value;

            var reqReview = new XMLHttpRequest();
            var reviewURL = "sfreviews.pl?author=" + selInitials;
            reqReview.open("GET", reviewURL);
            reqReview.send(null);

            reqReview.onreadystatechange = function() {
               if (this.readyState == 4) {
                  if (this.status == 200) {

                     // Process the request response
                     var divContainer = document.getElementById("review");
                     divContainer.innerHTML = this.responseText;
                 
                  }
               }
            }

         }

         archiveDIV.appendChild(authorSelect);                
         }
      }
   }


   var pods = document.getElementById("podcasts");

   // Request object for the headlines feed
   var reqPod = new XMLHttpRequest();
   reqPod.open("GET", "sfpod.xml");
   reqPod.send(null);

   reqPod.onreadystatechange = function() {
      if (this.readyState == 4) {
         if (this.status == 200) {
            // Retrieve the RSS feed
            var podDoc = this.responseXML;
            var rssDoc = new RSSFeed(podDoc);
            rssDoc.parseToHTML(pods);
         }
      }
   }

}


/* Constructor function for RSS news feeds */
function RSSFeed(xmlDoc) {

   // Retrieve the news feed title, link, and description 
   var channel = xmlDoc.getElementsByTagName("channel")[0];
   var title = channel.getElementsByTagName("title")[0];
   var link = channel.getElementsByTagName("link")[0];
   var description = channel.getElementsByTagName("description")[0];

   this.title = title.firstChild.nodeValue;
   this.link = link.firstChild.nodeValue;
   this.description = description.firstChild.nodeValue;

   /* Constructor function for an RSS news item */
   function RSSItem(item) {
      var title = item.getElementsByTagName("title")[0];
      var link = item.getElementsByTagName("link")[0];
      var description = item.getElementsByTagName("description")[0];

      this.title = title.firstChild.nodeValue;
      this.link = link.firstChild.nodeValue;
      this.description = description.firstChild.nodeValue;
   }

   // Create an array of news feed items
   this.items = new Array();

   var feedItems = channel.getElementsByTagName("item");
   for (var i = 0; i < feedItems.length; i++) {
      var feedItem = new RSSItem(feedItems[i]);
      this.items.push(feedItem);
   }

}


/* Method to write the RSSFeed document to an HTML fragment */
RSSFeed.prototype.parseToHTML = function(outputNode) {

   var fTitle = document.createElement("h1")
   var fTitleLink = document.createElement("a");
   fTitleLink.href = this.link;
   fTitleLink.innerHTML = this.title;
   fTitle.appendChild(fTitleLink);
   outputNode.appendChild(fTitle);

   var fPara = document.createElement("p");
   fPara.innerHTML = this.description;

   outputNode.appendChild(fPara);

   for (var i = 0; i < this.items.length; i++) {
      var iTitle = document.createElement("h2");
      var iTitleLink = document.createElement("a");
      iTitleLink.innerHTML = this.items[i].title;
      iTitleLink.href = this.items[i].link;

      iTitle.appendChild(iTitleLink);
      outputNode.appendChild(iTitle);

      var iPara = document.createElement("p");
      iPara.innerHTML = this.items[i].description;
      outputNode.appendChild(iPara);
   }

}
